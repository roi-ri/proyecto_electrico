#include "infrastructure/python/PythonPlotEngine.h"

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

#ifdef BATVIEW_ENABLE_EMBEDDED_PYTHON
#include <Python.h>
#endif

namespace batview::infrastructure::python {

namespace {

#ifdef BATVIEW_ENABLE_EMBEDDED_PYTHON

std::once_flag g_pythonInitFlag;
bool g_pythonReady = false;
std::string g_pythonInitError;

std::once_flag g_pythonHelperFlag;
bool g_pythonHelpersLoaded = false;
std::string g_pythonHelperError;

struct PythonRuntimeLayout {
    std::string home;
    std::string stdlib;
    std::string sitePackages;
};

std::filesystem::path ExecutablePath() {
#ifdef _WIN32
    std::vector<char> buffer(MAX_PATH);
    DWORD size = 0;
    while (true) {
        size = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (size == 0) {
            return {};
        }
        if (size < buffer.size()) {
            return std::filesystem::path(std::string(buffer.data(), size));
        }
        buffer.resize(buffer.size() * 2);
    }
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }
    std::error_code error;
    return std::filesystem::weakly_canonical(std::filesystem::path(buffer.data()), error);
#else
    std::vector<char> buffer(1024);
    while (true) {
        const ssize_t size = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (size < 0) {
            return {};
        }
        if (static_cast<std::size_t>(size) < buffer.size()) {
            return std::filesystem::path(std::string(buffer.data(), static_cast<std::size_t>(size)));
        }
        buffer.resize(buffer.size() * 2);
    }
#endif
}

bool PathExists(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return false;
    }

    std::error_code error;
    return std::filesystem::exists(std::filesystem::path(path), error) && !error;
}

bool PathExists(const std::filesystem::path& path) {
    std::error_code error;
    return !path.empty() && std::filesystem::exists(path, error) && !error;
}

PythonRuntimeLayout ResolveRuntimeLayoutFromRoot(const std::filesystem::path& root) {
    PythonRuntimeLayout layout;
    if (!PathExists(root)) {
        return layout;
    }

    layout.home = root.string();

#ifdef _WIN32
    const auto stdlib = root / "Lib";
    if (PathExists(stdlib)) {
        layout.stdlib = stdlib.string();
        const auto sitePackages = stdlib / "site-packages";
        if (PathExists(sitePackages)) {
            layout.sitePackages = sitePackages.string();
        }
    }
#else
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(root / "lib", error)) {
        if (!entry.is_directory()) {
            continue;
        }
        const auto dirname = entry.path().filename().string();
        if (dirname.rfind("python3.", 0) == 0) {
            layout.stdlib = entry.path().string();
            const auto sitePackages = entry.path() / "site-packages";
            if (PathExists(sitePackages)) {
                layout.sitePackages = sitePackages.string();
            }
            break;
        }
    }
#endif

    if (layout.stdlib.empty()) {
        layout.home.clear();
    }
    return layout;
}

PythonRuntimeLayout ResolvePythonRuntimeLayout() {
    std::vector<std::filesystem::path> candidates;
    const auto exePath = ExecutablePath();
    if (!exePath.empty()) {
        const auto exeDir = exePath.parent_path();
        candidates.push_back(exeDir / "python");
#ifdef __APPLE__
        candidates.push_back(exeDir.parent_path() / "Resources" / "python");
#endif
    }

    if (PathExists(BATVIEW_EMBEDDED_PYTHON_HOME)) {
        candidates.emplace_back(BATVIEW_EMBEDDED_PYTHON_HOME);
    }

    for (const auto& candidate : candidates) {
        const auto layout = ResolveRuntimeLayoutFromRoot(candidate);
        if (!layout.home.empty()) {
            return layout;
        }
    }

    return {};
}

const char* kPythonHelpers = R"PY(
import io
from PIL import Image, ImageDraw
from openpyxl import Workbook

def _safe_range(values):
    if not values:
        return 0.0, 1.0
    vmin = min(values)
    vmax = max(values)
    if vmin == vmax:
        delta = 1.0 if vmin == 0 else abs(vmin) * 0.1
        return vmin - delta, vmax + delta
    padding = (vmax - vmin) * 0.08
    return vmin - padding, vmax + padding

def _project(value, src_min, src_max, dst_min, dst_max):
    if src_max == src_min:
        return (dst_min + dst_max) / 2.0
    ratio = (value - src_min) / (src_max - src_min)
    return dst_min + ratio * (dst_max - dst_min)

def batview_render_plot_png(x_values, y_values, width, height, x_label, y_label, title):
    width = max(int(width), 320)
    height = max(int(height), 220)
    image = Image.new('RGBA', (width, height), (13, 18, 24, 255))
    draw = ImageDraw.Draw(image)

    margin_left = 68
    margin_right = 20
    margin_top = 34
    margin_bottom = 42

    plot_left = margin_left
    plot_top = margin_top
    plot_right = width - margin_right
    plot_bottom = height - margin_bottom

    draw.rounded_rectangle((0, 0, width - 1, height - 1), radius=12, outline=(55, 64, 78, 255), width=1)
    draw.rectangle((plot_left, plot_top, plot_right, plot_bottom), outline=(72, 82, 94, 255), width=1)

    x_min, x_max = _safe_range(x_values)
    y_min, y_max = _safe_range(y_values)

    for idx in range(5):
        frac = idx / 4.0
        y = _project(frac, 0.0, 1.0, plot_bottom, plot_top)
        draw.line((plot_left, y, plot_right, y), fill=(35, 43, 52, 255), width=1)
        tick_value = y_min + (y_max - y_min) * frac
        draw.text((8, y - 7), f'{tick_value:.2f}', fill=(194, 202, 215, 255))

    for idx in range(5):
        frac = idx / 4.0
        x = _project(frac, 0.0, 1.0, plot_left, plot_right)
        draw.line((x, plot_top, x, plot_bottom), fill=(35, 43, 52, 255), width=1)
        tick_value = x_min + (x_max - x_min) * frac
        draw.text((x - 18, plot_bottom + 8), f'{tick_value:.2f}', fill=(194, 202, 215, 255))

    draw.text((plot_left, 8), title, fill=(229, 236, 246, 255))
    draw.text((plot_left, height - 22), x_label, fill=(146, 200, 255, 255))
    draw.text((8, 8), y_label, fill=(255, 190, 112, 255))

    if x_values and y_values:
        points = []
        for x_value, y_value in zip(x_values, y_values):
            px = _project(x_value, x_min, x_max, plot_left, plot_right)
            py = _project(y_value, y_min, y_max, plot_bottom, plot_top)
            points.append((px, py))

        if len(points) == 1:
            px, py = points[0]
            draw.ellipse((px - 3, py - 3, px + 3, py + 3), fill=(90, 255, 180, 255))
        else:
            draw.line(points, fill=(90, 255, 180, 255), width=2)

    stream = io.BytesIO()
    image.save(stream, format='PNG')
    return stream.getvalue()

def batview_export_plot_xlsx(file_path, x_label, y_label, x_values, y_values):
    workbook = Workbook()
    sheet = workbook.active
    sheet.title = 'plot'
    sheet.append([x_label, y_label])
    for x_value, y_value in zip(x_values, y_values):
        sheet.append([x_value, y_value])
    workbook.save(file_path)

def batview_export_measurements_xlsx(file_path, measurements):
    workbook = Workbook()
    sheet = workbook.active
    sheet.title = 'measurements'
    sheet.append(['TimestampMs', 'Voltage', 'Current', 'State', 'CompletedCycles'])
    for row in measurements:
        sheet.append(row)
    workbook.save(file_path)
)PY";

bool SetPythonError(std::string& outError, const std::string& fallback) {
    outError = fallback;
    if (!PyErr_Occurred()) {
        return false;
    }

    PyObject* type = nullptr;
    PyObject* value = nullptr;
    PyObject* traceback = nullptr;
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    if (value != nullptr) {
        PyObject* valueStr = PyObject_Str(value);
        if (valueStr != nullptr) {
            outError = PyUnicode_AsUTF8(valueStr);
            Py_DECREF(valueStr);
        }
    }

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
    return true;
}

PyObject* VectorToPyList(const std::vector<double>& values) {
    PyObject* list = PyList_New(static_cast<Py_ssize_t>(values.size()));
    if (list == nullptr) {
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < static_cast<Py_ssize_t>(values.size()); ++i) {
        PyObject* value = PyFloat_FromDouble(values[static_cast<std::size_t>(i)]);
        if (value == nullptr) {
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, value);
    }

    return list;
}

PyObject* MeasurementsToPyList(const std::vector<core::models::Measurement>& measurements) {
    PyObject* list = PyList_New(static_cast<Py_ssize_t>(measurements.size()));
    if (list == nullptr) {
        return nullptr;
    }

    for (Py_ssize_t i = 0; i < static_cast<Py_ssize_t>(measurements.size()); ++i) {
        const auto& measurement = measurements[static_cast<std::size_t>(i)];
        PyObject* row = Py_BuildValue("(Kddii)",
                                      static_cast<unsigned long long>(measurement.timestampMs),
                                      measurement.voltage,
                                      measurement.current,
                                      measurement.state,
                                      measurement.completedCycles);
        if (row == nullptr) {
            Py_DECREF(list);
            return nullptr;
        }
        PyList_SET_ITEM(list, i, row);
    }

    return list;
}

#endif

} // namespace

PythonPlotEngine::PythonPlotEngine() = default;
PythonPlotEngine::~PythonPlotEngine() = default;

bool PythonPlotEngine::EnsureInitialized(std::string& outError) const {
#ifndef BATVIEW_ENABLE_EMBEDDED_PYTHON
    outError = "La app fue compilada sin soporte para Python embebido.";
    return false;
#else
    std::call_once(g_pythonInitFlag, []() {
        const auto runtimeLayout = ResolvePythonRuntimeLayout();
        PyStatus status = PyStatus_Ok();
        PyConfig config;
        PyConfig_InitPythonConfig(&config);
        config.parse_argv = 0;

        if (runtimeLayout.home.empty()) {
            g_pythonInitError =
                "No se encontro el runtime Python embebido. Verifica BATVIEW_EMBEDDED_PYTHON_HOME en el paquete final.";
            g_pythonReady = false;
            PyConfig_Clear(&config);
            return;
        }

        if (runtimeLayout.stdlib.empty()) {
            g_pythonInitError =
                "No se encontro la libreria estandar de Python embebido. El paquete de la app esta incompleto.";
            g_pythonReady = false;
            PyConfig_Clear(&config);
            return;
        }

        status = PyConfig_SetBytesString(&config, &config.home, runtimeLayout.home.c_str());
        if (!PyStatus_Exception(status)) {
            status = PyConfig_SetBytesString(&config, &config.program_name, "batView");
        }
        if (!PyStatus_Exception(status)) {
            status = Py_InitializeFromConfig(&config);
        }

        if (PyStatus_Exception(status)) {
            g_pythonInitError = status.err_msg ? status.err_msg : "No se pudo inicializar Python embebido.";
            g_pythonReady = false;
        } else {
            g_pythonReady = true;
            const std::string pythonPathSetup =
                "import sys\n"
                "paths = [r'" + runtimeLayout.stdlib + "', r'" + runtimeLayout.sitePackages + "']\n"
                "for path in paths:\n"
                "    if path and path not in sys.path:\n"
                "        sys.path.insert(0, path)\n";
            PyRun_SimpleString(pythonPathSetup.c_str());
        }

        PyConfig_Clear(&config);
    });

    outError = g_pythonInitError;
    return g_pythonReady;
#endif
}

bool PythonPlotEngine::EnsureHelpersLoaded(std::string& outError) const {
#ifndef BATVIEW_ENABLE_EMBEDDED_PYTHON
    outError = "La app fue compilada sin soporte para Python embebido.";
    return false;
#else
    if (!EnsureInitialized(outError)) {
        return false;
    }

    std::call_once(g_pythonHelperFlag, []() {
        PyGILState_STATE gil = PyGILState_Ensure();
        if (PyRun_SimpleString(kPythonHelpers) != 0) {
            SetPythonError(g_pythonHelperError, "No se pudo cargar el helper Python de gráficos.");
            g_pythonHelpersLoaded = false;
        } else {
            g_pythonHelpersLoaded = true;
        }
        PyGILState_Release(gil);
    });

    outError = g_pythonHelperError;
    return g_pythonHelpersLoaded;
#endif
}

core::services::PlotRenderResult PythonPlotEngine::RenderPlotPng(const std::vector<double>& xValues,
                                                                 const std::vector<double>& yValues,
                                                                 int width,
                                                                 int height,
                                                                 const std::string& xLabel,
                                                                 const std::string& yLabel,
                                                                 const std::string& title) const {
    core::services::PlotRenderResult result;
    if (!EnsureHelpersLoaded(result.error)) {
        return result;
    }

#ifdef BATVIEW_ENABLE_EMBEDDED_PYTHON
    PyGILState_STATE gil = PyGILState_Ensure();
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* globals = PyModule_GetDict(mainModule);
    PyObject* function = PyDict_GetItemString(globals, "batview_render_plot_png");
    if (function == nullptr || !PyCallable_Check(function)) {
        result.error = "No se encontro la funcion Python de render.";
        PyGILState_Release(gil);
        return result;
    }

    PyObject* xList = VectorToPyList(xValues);
    PyObject* yList = VectorToPyList(yValues);
    if (xList == nullptr || yList == nullptr) {
        result.error = "No se pudo preparar la serie para Python.";
        Py_XDECREF(xList);
        Py_XDECREF(yList);
        PyGILState_Release(gil);
        return result;
    }

    PyObject* args = Py_BuildValue("(OOiisss)",
                                   xList,
                                   yList,
                                   width,
                                   height,
                                   xLabel.c_str(),
                                   yLabel.c_str(),
                                   title.c_str());
    Py_DECREF(xList);
    Py_DECREF(yList);

    PyObject* pyResult = PyObject_CallObject(function, args);
    Py_DECREF(args);

    if (pyResult == nullptr) {
        SetPythonError(result.error, "Python no pudo renderizar la imagen del plot.");
        PyGILState_Release(gil);
        return result;
    }

    char* buffer = nullptr;
    Py_ssize_t size = 0;
    if (PyBytes_AsStringAndSize(pyResult, &buffer, &size) != 0) {
        SetPythonError(result.error, "Python devolvio una imagen PNG inválida.");
        Py_DECREF(pyResult);
        PyGILState_Release(gil);
        return result;
    }

    result.pngBytes.assign(buffer, buffer + size);
    result.success = true;
    Py_DECREF(pyResult);
    PyGILState_Release(gil);
#endif

    return result;
}

bool PythonPlotEngine::ExportSeriesToXlsx(const std::string& filePath,
                                          const std::string& xLabel,
                                          const std::string& yLabel,
                                          const std::vector<double>& xValues,
                                          const std::vector<double>& yValues,
                                          std::string& outError) const {
    if (!EnsureHelpersLoaded(outError)) {
        return false;
    }

#ifdef BATVIEW_ENABLE_EMBEDDED_PYTHON
    PyGILState_STATE gil = PyGILState_Ensure();
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* globals = PyModule_GetDict(mainModule);
    PyObject* function = PyDict_GetItemString(globals, "batview_export_plot_xlsx");
    if (function == nullptr || !PyCallable_Check(function)) {
        outError = "No se encontro la funcion Python de exportación XLSX.";
        PyGILState_Release(gil);
        return false;
    }

    PyObject* xList = VectorToPyList(xValues);
    PyObject* yList = VectorToPyList(yValues);
    if (xList == nullptr || yList == nullptr) {
        outError = "No se pudo preparar la serie para exportar a XLSX.";
        Py_XDECREF(xList);
        Py_XDECREF(yList);
        PyGILState_Release(gil);
        return false;
    }

    PyObject* args = Py_BuildValue("(sssOO)",
                                   filePath.c_str(),
                                   xLabel.c_str(),
                                   yLabel.c_str(),
                                   xList,
                                   yList);
    Py_DECREF(xList);
    Py_DECREF(yList);

    PyObject* pyResult = PyObject_CallObject(function, args);
    Py_DECREF(args);

    if (pyResult == nullptr) {
        SetPythonError(outError, "Python no pudo exportar el XLSX.");
        PyGILState_Release(gil);
        return false;
    }

    Py_DECREF(pyResult);
    PyGILState_Release(gil);
#endif

    return true;
}

bool PythonPlotEngine::ExportMeasurementsToXlsx(const std::string& filePath,
                                                const std::vector<core::models::Measurement>& measurements,
                                                std::string& outError) const {
    if (!EnsureHelpersLoaded(outError)) {
        return false;
    }

#ifdef BATVIEW_ENABLE_EMBEDDED_PYTHON
    PyGILState_STATE gil = PyGILState_Ensure();
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* globals = PyModule_GetDict(mainModule);
    PyObject* function = PyDict_GetItemString(globals, "batview_export_measurements_xlsx");
    if (function == nullptr || !PyCallable_Check(function)) {
        outError = "No se encontro la funcion Python de exportación XLSX de mediciones.";
        PyGILState_Release(gil);
        return false;
    }

    PyObject* measurementsList = MeasurementsToPyList(measurements);
    if (measurementsList == nullptr) {
        outError = "No se pudieron preparar las mediciones para exportar a XLSX.";
        PyGILState_Release(gil);
        return false;
    }

    PyObject* args = Py_BuildValue("(sO)", filePath.c_str(), measurementsList);
    Py_DECREF(measurementsList);

    PyObject* pyResult = PyObject_CallObject(function, args);
    Py_DECREF(args);

    if (pyResult == nullptr) {
        SetPythonError(outError, "Python no pudo exportar el XLSX de mediciones.");
        PyGILState_Release(gil);
        return false;
    }

    Py_DECREF(pyResult);
    PyGILState_Release(gil);
#endif

    return true;
}

} // namespace batview::infrastructure::python
