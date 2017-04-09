// glfwGetKey() stuck key state, GLFW_PRESS persists after onblur - should clear key state on window losing focus https://github.com/kripken/emscripten/issues/5122
/*/
 emcc -s USE_GLFW=3 test_glfw_get_key_stuck.c -o test_glfw_get_key_stuck.html

 clang test_glfw_get_key_stuck.c -I ../deps/glfw/include/ ../deps/glfw/build/src/libglfw3.a -framework Cocoa -framework CoreGraphics -framework IOKit -framework OpenGL -framework CoreVideo
 */
#include <stdio.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

GLFWwindow *window;

static int last_state = -1;
void render() {
    // http://www.glfw.org/docs/latest/input_guide.html#input_key
    int state = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (last_state != state) { // to not spam console
        last_state = state;
        printf("glfwGetKey says space pressed? %d\n", state);

    }

    // Red while space is pressed, green while not
    if (state) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    } else {
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
}

#ifdef __EMSCRIPTEN__
// Log these events for informational purposes - should they clear glfwGetKey states?
EM_BOOL on_focuspocus(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData) {
    switch(eventType) {
        case EMSCRIPTEN_EVENT_BLUR:
            printf("blur\n");
            break;
        case EMSCRIPTEN_EVENT_FOCUS:
            printf("focus\n");
            break;
        case EMSCRIPTEN_EVENT_FOCUSIN:
            printf("focusin\n");
            break;
        case EMSCRIPTEN_EVENT_FOCUSOUT:
            printf("focusout\n");
            break;
        default:
            printf("focus event %d\n", eventType);
            break;
    }

    //printf("eventType=%d, nodeName=%s, id=%s\n", eventType, focusEvent->nodeName, focusEvent->id);
    return EM_FALSE;
}

#endif

int main() {
    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "test_glfw_get_key_stuck", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

#ifdef __EMSCRIPTEN__
    emscripten_set_blur_callback(NULL, NULL, EM_TRUE, on_focuspocus);
    emscripten_set_focus_callback(NULL, NULL, EM_TRUE, on_focuspocus);
    emscripten_set_focusin_callback(NULL, NULL, EM_TRUE, on_focuspocus);
    emscripten_set_focusout_callback(NULL, NULL, EM_TRUE, on_focuspocus);

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
