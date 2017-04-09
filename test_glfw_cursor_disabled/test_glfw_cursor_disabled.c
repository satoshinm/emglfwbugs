/*/
 emcc -s USE_GLFW=3 test_glfw_cursor_disabled.c -o test_glfw_cursor_disabled.html
 */
#include <stdio.h>
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

GLFWwindow *window;

static int last_cursor_disabled = -1;
void render() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int cursor_disabled = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

    if (cursor_disabled != last_cursor_disabled) { // Only log changes as to not spam console
        last_cursor_disabled = cursor_disabled;

        printf("GLFW_CURSOR_DISABLED? %d\n", cursor_disabled);
        // This is expected to represent whether Pointer Lock is enabled/disabled, but it doesn't in 
        // emscripten 1.37.0. Instead, cursor_disabled is 1 since GLFW_CURSOR_DISABLED was set, but
        // it should be 0 since Pointer Lock was not acquired. This is the behavior apps would expect.
    }
}

#ifdef __EMSCRIPTEN__
EM_BOOL on_pointerlockchange(int eventType, const EmscriptenPointerlockChangeEvent *event, void *userData) {
    printf("pointerlockchange, isActive=%d\n", event->isActive);
    /* This is the application-level workaround to sync HTML5 Pointer Lock with glfw cursor state.
    if (!pointerlockChangeEvent->isActive) {
        printf("pointerlockchange deactivated, so enabling cursor\n");
        glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    */
    return 0;
}
#endif

int main() {
    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "test_glfw_cursor_disabled", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Try to disable the cursor - this attempts to activate Pointer Lock, but in the browser,
    // activating Pointer Lock requires a user action, so it actually does not activate it.
    // That is, this call fails, but it returns void so we cannot check here.
    //
    // http://www.glfw.org/docs/latest/group__input.html#gaa92336e173da9c8834558b54ee80563b
    // "GLFW_CURSOR_DISABLED hides and grabs the cursor, providing virtual and unlimited cursor movement.
    // This is useful for implementing for example 3D camera controls."
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifdef __EMSCRIPTEN__
    emscripten_set_pointerlockchange_callback(NULL, NULL, 0, on_pointerlockchange);
    emscripten_set_main_loop(render, 0, 1);
#else
    // TODO
#endif

    glfwTerminate();
    return 0;
}
