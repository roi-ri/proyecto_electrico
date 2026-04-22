#pragma once

#include <wx/frame.h>

namespace batview::ui::frames {

/**
 * @brief Ventana principal de la aplicación.
 *
 * En esta etapa inicial actúa como contenedor de la interfaz base.
 * No debe contener lógica de negocio.
 */
class MainFrame : public wxFrame {
public:
    explicit MainFrame(const wxString& title);

private:
    void BuildLayout();
};

} // namespace batview::ui::frames
