#include <glad/glad.h> // OpenGL function loader; required for modern GL calls. Importance: 9/10

#include <assert.h> // Debug assertions (not used here). Importance: 3/10
#include <math.h> // C math helpers (not used directly here). Importance: 2/10
#include <stdio.h> // fprintf for error output. Importance: 6/10
#include <stdlib.h> // General C stdlib (not used directly here). Importance: 2/10

#include <vector> // std::vector for CPU-side vertex storage. Importance: 8/10

#include <imgui/imgui.h> // ImGui core API. Importance: 5/10
#include <imgui/imgui_impl_opengl3.h> // ImGui OpenGL renderer backend. Importance: 4/10
#include <imgui/imgui_impl_sdl3.h> // ImGui SDL3 platform backend. Importance: 4/10

#include <SDL3/SDL.h> // Windowing, events, input. Importance: 8/10
#include <SDL3/SDL_opengl.h> // SDL <-> OpenGL integration. Importance: 7/10

#include "../ramen/ramen.h" // Framework bootstrap/wrapper utilities. Importance: 7/10
#include "../ramen/rgl_camera.h" // Camera utilities. Importance: 8/10
#include "../ramen/rgl_defines.h" // Shared macros like TO_RAD. Importance: 6/10
#include "../ramen/rgl_math.h" // Matrix/vector math helpers. Importance: 9/10
#include "../ramen/rgl_model.h" // Model loading and Vertex definition. Importance: 9/10
#include "../ramen/rgl_shader.h" // Shader wrapper class. Importance: 8/10

int main(int argc, char** argv) // Program entry point. Importance: 8/10
{
    Filesystem* pFS = Filesystem::Init(argc, argv, "assets"); // Initialize virtual filesystem and asset root. Importance: 8/10
    (void)pFS; // Silence unused variable warning in this task file. Importance: 3/10

    Ramen* pRamen = Ramen::Instance(); // Access singleton app context. Importance: 6/10
    pRamen->Init("GUI", 800, 600); // Create window and GL context. Importance: 9/10

    /* Load shaders. */
    Shader shader{}; // Shader program wrapper instance. Importance: 7/10
    if ( !shader.Load("shaders/task02.vert", "shaders/task02.frag") ) // Compile/link shader stages. Importance: 9/10
    {
        fprintf(stderr, "Could not load shader.\n"); // Print error when shader setup fails. Importance: 6/10
    }

    /* Load model data from disk */
    Model model{}; // Model wrapper instance. Importance: 7/10 a model is a collection of vertices loaded from an asset file, along with helper methods to access them. Importance: 8/10
    if ( !model.Load("models/stormtrooper.obj") ) // Parse OBJ and populate vertices. Importance: 9/10  obj is a common 3D model format that contains vertex positions, normals, UVs, and face definitions. Importance: 8/10
    {
        fprintf(stderr, "Could not load model file.\n"); // Print error when model loading fails. Importance: 7/10
    }

    std::vector<Vertex> vertices = model.GetVertices(); // Copy loaded vertices to local CPU container. Importance: 9/10 Otherwise we would have to call model.GetVertices() repeatedly during rendering which would be inefficient. Importance: 8/10 Rendering means issuing draw calls to GPU each frame. Importance: 9/10

    GLuint vbo = 0; // GPU vertex buffer object handle. Importance: 8/10 A VBO is a GPU resource that holds vertex data in GPU memory for efficient rendering. Importance: 9/10
    GLuint vao = 0; // Vertex array object handle (input layout state). Importance: 8/10

    // TODO: Create a buffer on GPU and upload the model's vertices.
    glCreateBuffers(1, &vbo); // Create one buffer object name. Importance: 8/10
    GLsizeiptr vertexBufferSize = static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)); // Compute total upload size in bytes. Importance: 9/10
    glNamedBufferStorage(vbo, vertexBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT); // Allocate immutable GPU storage. Importance: 8/10 We need immutable storage for best performance, but we also need to update it with our vertex data, so we use the DYNAMIC_STORAGE_BIT flag to allow updates via glNamedBufferSubData. Importance: 9/10
    glNamedBufferSubData(vbo, 0, vertexBufferSize, vertices.data()); // Upload CPU vertex data into VBO. Importance: 9/10 So the data is accessable from the GPU for rendering. Importance: 9/10

    // TODO: Create vertex layout via VAO.
    glCreateVertexArrays(1, &vao); // Create VAO object. Importance: 8/10
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex)); // Bind VBO to VAO binding slot 0 with stride Vertex size. Importance: 10/10
    glVertexArrayAttribBinding(vao, 0, 0); // Route attribute location 0 to binding slot 0. Importance: 9/10
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0); // Define attribute 0 as vec3 position at byte offset 0. Importance: 10/10
    glEnableVertexArrayAttrib(vao, 0); // Enable attribute location 0. Importance: 9/10 This is important because disabled attributes are not sent to the vertex shader, so if we forget this step we won't see any geometry. Importance: 10/10

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 0.0f, 20.0f }); // Place camera along +Z so model is visible. Importance: 8/10

    /* Model mat*/
    Mat4f modelMat = Mat4f::Identity(); // No model transform in this task. Importance: 8/10

    /*  Gobal GL states */
    glEnable(GL_DEPTH_TEST); // Enable depth testing. Importance: 9/10
    glDepthFunc(GL_LESS); // Pass fragments with smaller depth values. Importance: 7/10
    glEnable(GL_CULL_FACE); // Enable back-face culling. Importance: 7/10
    glCullFace(GL_BACK); // Cull back-facing triangles. Importance: 7/10
    glFrontFace(GL_CCW); // Treat counter-clockwise winding as front face. Importance: 7/10

    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabled; 0 = VSync disabled */ // Synchronize frame swap to monitor refresh. Importance: 5/10

    /* Main loop */
    bool isRunning = true; // Controls app lifetime. Importance: 6/10
    while ( isRunning ) // Main event/render loop. Importance: 9/10
    {
        SDL_Event e; // Event container for SDL polling. Importance: 6/10
        while ( SDL_PollEvent(&e) ) // Drain all queued input/window events. Importance: 8/10
        {
            ImGui_ImplSDL3_ProcessEvent(&e); // Forward event to ImGui backend. Importance: 5/10

            if ( e.type == SDL_EVENT_QUIT ) // User requested window close. Importance: 7/10
            {
                isRunning = false; // Exit main loop cleanly. Importance: 7/10
            }

            if ( e.type == SDL_EVENT_KEY_DOWN ) // React to key press events. Importance: 6/10
            {
                switch ( e.key.key ) // Branch per key code. Importance: 6/10
                {
                case SDLK_ESCAPE: // Escape closes app. Importance: 6/10
                {
                    isRunning = false; // Mark loop for termination. Importance: 7/10
                }
                break;

                default: // Ignore other keys in this task. Importance: 4/10
                {
                }
                }
            }
        }

        /* Query new frame dimensions */
        int windowWidth, windowHeight; // Current framebuffer/window dimensions. Importance: 6/10
        SDL_GetWindowSize(pRamen->GetWindow(), &windowWidth, &windowHeight); // Read actual window size each frame. Importance: 8/10

        /* Adjust viewport and perspective projection accordingly. */
        glViewport(0, 0, windowWidth, windowHeight); // Map NDC to full window area. Importance: 9/10

        /* View mat */
        Mat4f viewMat = LookAt(
            camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp()); // Build view matrix from camera basis. Importance: 9/10

        /* Projection mat */
        float aspect  = (float)windowWidth / (float)windowHeight; // Compute width/height ratio for perspective. Importance: 9/10
        Mat4f projMat = PerspectiveProjection(TO_RAD(70.0f), aspect, 0.01f, 500.0f); // Build perspective projection matrix. Importance: 9/10

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame(); // Prepare ImGui OpenGL renderer state for this frame. Importance: 4/10
        ImGui_ImplSDL3_NewFrame(); // Prepare ImGui SDL platform state for this frame. Importance: 4/10
        ImGui::NewFrame(); // Begin ImGui command recording. Importance: 4/10

        // NOTE: Bonus: You can uncomment this and check out
        // what the UI library can do. We will work with it later.
        // ImGui::ShowDemoWindow();

        /* ImGUI Rendering */
        ImGui::Render(); // Finalize ImGui draw command list. Importance: 4/10

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear frame/depth buffers from previous frame. Importance: 9/10
        glClearColor(1.0f, 0.95f, 0.0f, 1.0f); // Set clear color value (applies on next clear call). Importance: 5/10

        shader.Use(); // Bind shader program for subsequent draw calls. Importance: 9/10

        glBindVertexArray(vao); // Bind VAO with vertex layout and bound VBO. Importance: 10/10 This is important, because there could exist multiple VAOs and here it is specified which one is used for the next DrawCall

        glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data()); // Upload model matrix to uniform location 0. Importance: 9/10
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data()); // Upload view matrix to uniform location 1. Importance: 9/10
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data()); // Upload projection matrix to uniform location 2. Importance: 9/10

        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(model.NumVertices())); // Draw all vertices as triangle list. Importance: 10/10

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render ImGui on top of 3D content. Importance: 4/10

        SDL_GL_SwapWindow(pRamen->GetWindow()); // Present backbuffer to screen. Importance: 9/10
    }

    /* GL Resources shutdown. */
    shader.Delete(); // Delete shader program resources. Importance: 7/10

    glDeleteVertexArrays(1, &vao); // Delete VAO resource. Importance: 7/10
    glDeleteBuffers(1, &vbo); // Delete VBO resource. Importance: 7/10

    /* Ramen Shutdown */
    pRamen->Shutdown(); // Shutdown framework/window/context cleanly. Importance: 8/10

    return 0; // Exit success. Importance: 6/10
}
