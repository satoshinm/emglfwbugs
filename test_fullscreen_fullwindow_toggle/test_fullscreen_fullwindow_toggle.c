// emscripten_request_fullscreen_strategy() fails where EM_ASM(Module.requestFullscreen(1, 1)) succeeds, fullscreen request deferred from glfwSetKeyCallback https://github.com/kripken/emscripten/issues/5124
/*/
 emcc -s USE_GLFW=3 test_fullscreen_fullwindow_toggle.c -o test_fullscreen_fullwindow_toggle.html

 clang test_fullscreen_fullwindow_toggle.c -I ../deps/glfw/include/ ../deps/glfw/build/src/libglfw3.a -framework Cocoa -framework CoreGraphics -framework IOKit -framework OpenGL -framework CoreVideo
 */
#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

GLFWwindow *window;

void render() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

#ifdef __EMSCRIPTEN__
EM_BOOL on_canvassize_changed(int eventType, const void *reserved, void *userData)
{
    /* TODO: ?
  int w, h, fs;
  emscripten_get_canvas_size(&w, &h, &fs);
  double cssW, cssH;
  emscripten_get_element_css_size(0, &cssW, &cssH);
  printf("Canvas resized: WebGL RTT size: %dx%d, canvas CSS size: %02gx%02g\n", w, h, cssW, cssH);

  glfwSetWindowSize(g->window, w, h);
  */
  return 0;
}
#endif

static int inFullscreen = 0;
static int wasFullscreen = 0;
void windowSizeCallback(GLFWwindow* window, int width, int height) {
#ifdef __EMSCRIPTEN__
  int isInFullscreen = EM_ASM_INT_V(return !!(document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement));
  if (isInFullscreen && !wasFullscreen) {
    printf("Successfully transitioned to fullscreen mode!\n");
    wasFullscreen = isInFullscreen;
  }

  if (wasFullscreen && !isInFullscreen) {
    printf("Exited fullscreen. Test succeeded.\n");
    wasFullscreen = isInFullscreen;
    //emscripten_cancel_main_loop();
    return;
  }
#endif
}

#ifdef __EMSCRIPTEN__

// Emscripten's "soft fullscreen" = maximizes the canvas in the browser client area, wanted to toggle soft/hard fullscreen
void maximize_canvas() {
    EmscriptenFullscreenStrategy strategy = {
        .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
        .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
        .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT, // or EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST
        .canvasResizedCallback = on_canvassize_changed,
        .canvasResizedCallbackUserData = NULL
    };

    EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen("#canvas", &strategy);
}

EM_BOOL fullscreen_change_callback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData) {
    printf("fullscreen_change_callback, isFullscreen=%d\n", event->isFullscreen);

    if (!event->isFullscreen) {
        // Go back to windowed mode with full-sized <canvas>, when user escapes out (instead of F11)
        maximize_canvas();
    }

    return EM_TRUE;
}


#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
int get_scale_factor(GLFWwindow *window) {
    int window_width, window_height;
    int buffer_width, buffer_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0) {
        return 0;
    }
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
    int result = buffer_width / window_width;
    result = MAX(1, result);
    result = MIN(2, result);
    return result;
}

GLFWmonitor *fullscreen_monitor;
int fullscreen_width, fullscreen_height;
int window_xpos, window_ypos, window_width, window_height;
void init_fullscreen_monitor_dimensions() {
    int mode_count;
    fullscreen_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *modes = glfwGetVideoModes(fullscreen_monitor, &mode_count);
    fullscreen_width = modes[mode_count - 1].width;
    fullscreen_height= modes[mode_count - 1].height;

    GLFWwindow *test_window = glfwCreateWindow(
        fullscreen_width, fullscreen_height, "Craft", NULL, NULL);
    int scale = get_scale_factor(test_window);
    glfwDestroyWindow(test_window);
    fullscreen_width /= scale;
    fullscreen_height /= scale;

#ifdef __EMSCRIPTEN__
    emscripten_set_fullscreenchange_callback(NULL, NULL, EM_TRUE, fullscreen_change_callback);
#endif
}


void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action && GLFW_PRESS && key == GLFW_KEY_F11) {
        printf("glfwSetKeyCallback: F11 pressed\n");
#ifdef __EMSCRIPTEN__
        // F11 toggles between fullscreen and fullwindow ("soft fullscreen") mode. The app starts
        // in fullwindow, a solid gray page. When fullscreen is entered with F11, the entire
        // screen should be solid gray. Returning to fullwindow with F11 should also show the same.

        EmscriptenFullscreenChangeEvent fsce;
        EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);

        if (!fsce.isFullscreen) {
            emscripten_exit_soft_fullscreen();

            // Enter fullscreen
            /* this returns 1=EMSCRIPTEN_RESULT_DEFERRED if EM_TRUE is given to defer
             * or -2=EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED if EM_FALSE
             * but the EM_ASM() JS works immediately?
            EmscriptenFullscreenStrategy strategy = {
                .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
                .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                .canvasResizedCallback = on_canvassize_changed,
                .canvasResizedCallbackUserData = NULL
            };
            EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(NULL, EM_FALSE, &strategy);
            printf("emscripten_request_fullscreen_strategy = %d\n", ret);
            */
            EM_ASM(Module.requestFullscreen(1, 1));
        } else {
            printf("Exiting fullscreen...\n");
            emscripten_exit_fullscreen();

            printf("Maximizing to canvas...\n");
            maximize_canvas();
        }
#else
    if (glfwGetWindowMonitor(window)) {
        glfwSetWindowMonitor(window, NULL, window_xpos, window_ypos, window_width, window_height, GLFW_DONT_CARE);
    } else {
        glfwGetWindowPos(window, &window_xpos, &window_ypos);
        glfwGetWindowSize(window, &window_width, &window_height);
        glfwSetWindowMonitor(window, fullscreen_monitor, 0, 0, fullscreen_width, fullscreen_height, GLFW_DONT_CARE);
    }
#endif
    }
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

	init_fullscreen_monitor_dimensions();
    window = glfwCreateWindow(640, 480, "test_fullscreen_fullwindow_toggle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Install callbacks
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetKeyCallback(window, on_key);

#ifdef __EMSCRIPTEN__
    maximize_canvas();
    emscripten_set_main_loop(render, 0, 1);
#else
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#endif

    glfwTerminate();
    return 0;
}
