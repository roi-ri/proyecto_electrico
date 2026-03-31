# Guía rápida: wxWidgets en macOS (VS Code + Terminal)

## 📌 Requisitos

Instalar herramientas básicas:

```bash
xcode-select --install
```

Instalar Homebrew (si no lo tienes):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

---

## 📦 Instalación de wxWidgets

### Opción rápida (recomendada al inicio)
```bash
brew install wxwidgets
```

Verificar instalación:

```bash
wx-config --version
wx-config --cxxflags
wx-config --libs
```

---

## 📁 Crear proyecto

```bash
mkdir mi_wx_app
cd mi_wx_app
touch main.cpp
```

---

## 🧠 Código mínimo (`main.cpp`)

```cpp
#include <wx/wx.h>

class MyApp : public wxApp {
public:
    bool OnInit() override;
};

class MyFrame : public wxFrame {
public:
    MyFrame()
        : wxFrame(nullptr, wxID_ANY, "Mi primera app wxWidgets",
                  wxDefaultPosition, wxSize(500, 300)) {}
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}
```

---

## ⚙️ Compilar desde terminal

```bash
g++ main.cpp $(wx-config --cxxflags --libs) -o mi_app
```

---

## ▶️ Ejecutar

```bash
./mi_app
```

---

## 🧪 Verificación rápida

```bash
wx-config --version
```

---

## 💻 Integración con VS Code

Crear carpeta:

```bash
mkdir .vscode
```

### `.vscode/tasks.json`

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build wxWidgets",
      "type": "shell",
      "command": "g++ main.cpp $(wx-config --cxxflags --libs) -o mi_app",
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": ["$gcc"]
    }
  ]
}
```

Compilar en VS Code:

Cmd + Shift + B

---

## ⚠️ Errores comunes

### wx/wx.h not found
Usar:
```bash
g++ main.cpp $(wx-config --cxxflags --libs) -o mi_app
```

### undefined symbols
Faltan librerías → usar wx-config

### VS Code marca errores
Es IntelliSense (ignorar)

---

## 🚀 Flujo recomendado

1. Instalar wxWidgets  
2. Crear main.cpp  
3. Compilar  
4. Ejecutar  
5. Integrar con VS Code  

---

## 📎 Comando clave

```bash
g++ main.cpp $(wx-config --cxxflags --libs) -o mi_app
```

