// 
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

static int inFullscreen = 0;
static int wasFullscreen = 0;
void windowSizeCallback(GLFWwindow* window, int width, int height) {
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
}

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


void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action && GLFW_PRESS && key == GLFW_KEY_F11) {
        printf("glfwSetKeyCallback: F11 pressed\n");
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
             *
            EmscriptenFullscreenStrategy strategy = {
                .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH,
                .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                .canvasResizedCallback = /on_canvassize_changed,
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
    }
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

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
