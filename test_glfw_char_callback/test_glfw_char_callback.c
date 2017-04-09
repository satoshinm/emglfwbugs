// 
/*/
 emcc -s USE_GLFW=3 test_glfw_char_callback.c -o test_glfw_char_callback.html

 clang test_glfw_char_callback.c -I ../deps/glfw/include/ ../deps/glfw/build/src/libglfw3.a -framework Cocoa -framework CoreGraphics -framework IOKit -framework OpenGL -framework CoreVideo
 */
#include <stdio.h>
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

/* http://www.glfw.org/docs/latest/input.html#input_char
 * "If you wish to offer regular text input, set a character callback."
 *
 * "The callback function receives Unicode code points for key events that
 * would have led to regular text input and generally behaves as a standard
 * text field on that platform."
 */
void on_char(GLFWwindow* window, unsigned int u) {
    // This should only give us actual characters the user would have typed,
    // and it does with glfw natively, but under emscripten, we are also called with:
    // Cmd-A gives us U+0061 'a' (on Firefox 53.0b9, but not Chrome)
    // Cmd-B gives us U+0062 'b' (on Firefox 53.0b9, but not Chrome)
    // Cmd-` gives us '`' (on Safari TP 27, but not Firefox or Chrome)
    // etc
    // Expected: should not even be called in those cases (Cmd pressed), like in native
    printf("on_char codepoint=U+%.4X '%c'\n", u, u);
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480, "test_glfw_char_callback", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetCharCallback(window, on_char);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render, 0, 1);
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
#endif

    glfwTerminate();
    return 0;
}
