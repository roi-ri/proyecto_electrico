#include "infrastructure/serial/SerialEnumerator.h"
#include <dirent.h>
#include <string>
#include <vector>

namespace batview::infrastructure::serial {

std::vector<std::string> SerialEnumerator::EnumeratePorts() {
    std::vector<std::string> ports;
    // Aquí implementamos la lógica para listar puertos seriales en macOS
    // (en Linux y Windows, puede necesitar código diferente)

    DIR* dir = opendir("/dev/");
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == 't' && entry->d_name[1] == 't') {  // Filtrar tty* como ejemplo
            ports.push_back(std::string("/dev/") + entry->d_name);
        }
    }
    closedir(dir);
    return ports;
}

} // namespace batview::infrastructure::serial
