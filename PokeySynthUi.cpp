#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/x.H>

#include <X11/Xlib.h>

class PokeySynthUi {
public:
    PokeySynthUi(LV2UI_Write_Function write_function,
                 LV2UI_Controller controller,
                 void *parentWindow);
    void portEvent(uint32_t port_index,
                   uint32_t buffer_size,
                   uint32_t format,
                   const void *buffer);

    Fl_Window *window;

private:
    LV2UI_Write_Function write_function;
    LV2UI_Controller controller;
};

PokeySynthUi::PokeySynthUi(LV2UI_Write_Function write_function,
                           LV2UI_Controller controller,
                           void *parentWindow) :
    write_function(write_function),
    controller(controller) {

    window = new Fl_Window(300,180);
    Fl_Box *box = new Fl_Box(20,40,260,100,"Hello, World!");
    box->box(FL_UP_BOX);
    box->labelsize(24);
    box->labelfont(FL_BOLD+FL_ITALIC);
    box->labeltype(FL_SHADOW_LABEL);
    window->end();
    window->show();

#ifdef __linux__

    Window w = fl_xid(window);

    //printf("debug: window = %llx\n", (unsigned long long ) w);
    //printf("debug: parentWindow = %llx\n", (unsigned long long) parentWindow);

    Fl::check();
    Fl::flush();
    usleep(100000);

    XUnmapWindow(fl_display, w);
    XSync(fl_display, False);
    usleep(100000);

    XReparentWindow(fl_display, w, (Window) parentWindow, 0,0);
    XMapWindow(fl_display, w);
    XSync(fl_display, False);

#elif _WIN32
    // windows code goes here, reparent HWND
    #error "WIN32 not implemented yet"
#else
    #error "Unsupported platform"
#endif
}

void PokeySynthUi::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer) {
}

// ****************************************************************************
//
// C API
//
static LV2UI_Handle instantiate(const struct LV2UI_Descriptor *descriptor,
                                const char *plugin_uri,
                                const char *bundle_path,
                                LV2UI_Write_Function write_function,
                                LV2UI_Controller controller,
                                LV2UI_Widget *widget,
                                const LV2_Feature *const *features) {

    if (strcmp (plugin_uri, "https://github.com/ivop/pokeysynth") != 0) {
        return nullptr;
    }

    void* parentWindow = nullptr;
    for (int i = 0; features[i]; ++i) {
        if (strcmp (features[i]->URI, LV2_UI__parent) == 0)
            parentWindow = features[i]->data;
    }

    if (!parentWindow) {
        fprintf(stderr, "Required feature LV2_UI__parent not provided\n");
        return nullptr;
    }

    PokeySynthUi *ui;
    try {
        ui = new PokeySynthUi(write_function, controller, parentWindow);
    }
    catch (std::exception& exc) {
        fprintf(stderr, "UI instantiation failed.\n");
        return nullptr;
    }

    *widget = reinterpret_cast<LV2UI_Widget>(fl_xid(ui->window));

    printf("debug: widget=%lx\n", fl_xid(ui->window));
    return (LV2UI_Handle) ui;
}

static void cleanup (LV2UI_Handle ui) {
    PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    if (psui) delete psui;
}

static void port_event(LV2UI_Handle ui,
                       uint32_t port_index,
                       uint32_t buffer_size,
                       uint32_t format,
                       const void *buffer) {
    PokeySynthUi* psui = static_cast<PokeySynthUi *>(ui);
    if (psui) psui->portEvent(port_index, buffer_size, format, buffer);
}

static int ui_idle(LV2UI_Handle ui) {
    //PokeySynthUi *psui = static_cast<PokeySynthUi *>(ui);
    Fl::check();
    Fl::flush();
    return 0;
}

static const void * extension_data (const char *uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    if (strcmp (uri, LV2_UI__idleInterface) == 0) return &idle;
    return nullptr;
}

static const LV2UI_Descriptor ui_descriptor = {
    "https://github.com/ivop/pokeysynth#ui",
    instantiate,
    cleanup,
    port_event,
    extension_data
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor * 	lv2ui_descriptor (uint32_t index) {
    switch (index) {
        case 0:     return &ui_descriptor;
        default:    return 0;
    }
}
