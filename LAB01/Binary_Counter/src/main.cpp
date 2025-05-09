//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 1
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

// Headers abaixo são específicos de C++
#include <string>
#include <fstream>
#include <sstream>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers locais, definidos na pasta "include/"
#include "utils.h"

// Estruturas relevantes para o programa
typedef struct {
    GLuint VAOid;
    GLuint size;
    GLenum draw_mode;
} VAOParams;

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
std::vector<VAOParams> BuildScene(GLuint count);
std::vector<GLubyte> DecimalToBinary(GLuint decimal_number, GLuint range);
GLuint BuildZero(GLuint vertex_array_object_id, std::vector<GLfloat> NDC_center, GLuint external_points_count);
GLuint BuildOne(GLuint vertex_array_object_id, std::vector<GLfloat> NDC_center);
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;

int main()
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 1200 colunas e 500 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(1200, 500, "INF01047 - 341810 - Pedro Lubaszewski Lima", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado.
    glfwSetKeyCallback(window, KeyCallback);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Medimos o tempo de execução inicial da aplicação
    GLuint t0 = (GLuint)glfwGetTime();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    GLuint counter = 0;
    std::vector<VAOParams> vao_params = BuildScene(counter);
    while (!glfwWindowShouldClose(window))
    {
        // Calcula se houve mudança de tempo entre cada loop
        GLuint t1 = (GLuint)glfwGetTime();
        if(t1 - t0 >= 1){
            counter++;
            if(counter > 15){
                counter = 0;
            }
            vao_params = BuildScene(counter);
            t0 = t1;
        }

        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima
        glClear(GL_COLOR_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos)
        glUseProgram(g_GpuProgramID);

        // Pedimos para a GPU rasterizar os vértices apontados pelo VAO como
        // triângulos
        for(VAOParams i : vao_params){
            glBindVertexArray(i.VAOid);
            glDrawElements(i.draw_mode, i.size, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...)
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Montar os triângulos para um respectivo valor de contagem
std::vector<VAOParams> BuildScene(GLuint count){
    // Definir dados vetoriais
    std::vector<VAOParams> params;

    // Definir tamanhos e posições básicas dos dígitos
    GLuint zero_external_points = 16;
    GLfloat center_step = 0.5f;
    GLfloat x_first_center = 0.75f;
    std::vector<GLfloat> NDC_center = {x_first_center, 0.0f, 0.0f, 1.0f};

    // Alocar as coordenadas dos pontos
    std::vector<GLfloat> NDC_coefficients;

    // Alocar as cores dos pontos
    std::vector<GLfloat> color_coefficients;

    // Alocar o vetor de índices
    std::vector<GLuint> indices;

    // Converter decimal para binário (little endian)
    std::vector<GLubyte> binary_count = DecimalToBinary(count, 4);

    // Construir os triângulos de acordo com os valores binários
    GLuint vertex_array_object_id;
    GLuint size;
    GLenum draw_mode;
    for(GLuint i : binary_count){

        // Atualizar estruturas do VAO
        glGenVertexArrays(1, &vertex_array_object_id);
        glBindVertexArray(vertex_array_object_id);

        // Construir triângulos de acordo com o dígito correto
        if(i == 0){
            draw_mode = GL_TRIANGLE_STRIP;
            size = BuildZero(vertex_array_object_id, NDC_center, zero_external_points);
        }else{
            draw_mode = GL_TRIANGLES;
            size = BuildOne(vertex_array_object_id, NDC_center);
        }
        // Armazenar os parâmetros da VAO
        params.push_back({vertex_array_object_id, size, draw_mode});

        // Atualizar o centro do próximo dígito
        NDC_center[0] -= center_step;

        // Bloquear o VAO
        glBindVertexArray(0);
    }

    return params;
}

// Converter decimal para um vetor binário (little endian)
std::vector<GLubyte> DecimalToBinary(GLuint decimal_number, GLuint range){
    std::vector<GLubyte> binary;

    for(GLuint i = 0; i < range; i++){
        binary.push_back(GLubyte(decimal_number % 2));
        decimal_number = decimal_number / 2;
    }

    return binary;
}

// Gerar pontos, cores e topologia do dígito zero
GLuint BuildZero(GLuint vertex_array_object_id, std::vector<GLfloat> NDC_center, GLuint external_points_count){
    // Definir informações dos vetores
    GLuint point_coords = 4;
    GLuint color_coding = 4;
    
    // Definir tamanhos básicos do dígito
    GLfloat x_minor_focus = 0.1f;
    GLfloat x_major_focus = 0.2f;
    GLfloat y_minor_focus = 0.6f;
    GLfloat y_major_focus = 0.7f;

    // Definir estruturas de VBOs
    std::vector<GLfloat> coordinates;
    std::vector<GLfloat> colors;
    std::vector<GLuint> topology;
    GLuint VBO_NDC_coefficients_id;
    GLuint VBO_color_coefficients_id;
    GLuint indices_id;

    // Definir coordenadas dos pontos
    GLfloat step = 2.0f * M_PI / external_points_count;
    GLfloat angle;
    for(GLuint i = 0; i < external_points_count; i++){
        angle = step*i;

        // Calcular os pontos internos
        coordinates.push_back(x_minor_focus * cosf(angle) + NDC_center[0]);
        coordinates.push_back(y_minor_focus * sinf(angle) + NDC_center[1]);
        coordinates.push_back(0.0f);
        coordinates.push_back(1.0f);

        // Calcular os pontos externos
        coordinates.push_back(x_major_focus * cosf(angle) + NDC_center[0]);
        coordinates.push_back(y_major_focus * sinf(angle) + NDC_center[1]);
        coordinates.push_back(0.0f);
        coordinates.push_back(1.0f);

        // Definir a cor vermelha para o dígito zero e os índices
        colors.push_back(1.0f);
        colors.push_back(0.0f);
        colors.push_back(0.0f);
        colors.push_back(1.0f);

        colors.push_back(1.0f);
        colors.push_back(0.0f);
        colors.push_back(0.0f);
        colors.push_back(1.0f);

        topology.push_back(GLubyte(2 * i));
        topology.push_back(GLubyte(2 * i + 1));
    }
    topology.push_back(0);
    topology.push_back(1);

    // Construir os VBOs para a posição geométrica

    glGenBuffers(1, &VBO_NDC_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, coordinates.size() * sizeof(GLfloat), coordinates.data(), GL_STATIC_DRAW);
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, point_coords, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir os VBOs para as informações de cores

    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, color_coding, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir o VBO para a topologia

    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, topology.size() * sizeof(GLuint), topology.data(), GL_STATIC_DRAW);

    GLuint size = 2 * external_points_count + 2;
    return size;
}

// Gerar pontos, cores e topologia do dígito um
GLuint BuildOne(GLuint vertex_array_object_id, std::vector<GLfloat> NDC_center){
    // Definir informações dos vetores
    GLuint point_coords = 4;
    GLuint color_coding = 4;

    // Definir tamanhos básicos do dígito
    GLfloat half_base = 0.05f;
    GLfloat half_height = 0.7f;
    GLfloat point_x = -0.1f + NDC_center[0];
    GLfloat point_y = 0.408f + NDC_center[1];
    
    // Definir estruturas de VBOs
    std::vector<GLfloat> coordinates;
    std::vector<GLfloat> colors;
    std::vector<GLuint> topology;
    GLuint VBO_NDC_coefficients_id;
    GLuint VBO_color_coefficients_id;
    GLuint indices_id;

    // Definir coordenadas dos pontos
    coordinates.push_back(NDC_center[0] - half_base);
    coordinates.push_back(NDC_center[1] - half_height + 0.008f);
    coordinates.push_back(0.0f);
    coordinates.push_back(1.0f);

    coordinates.push_back(NDC_center[0] + half_base);
    coordinates.push_back(NDC_center[1] - half_height + 0.008f);
    coordinates.push_back(0.0f);
    coordinates.push_back(1.0f);

    coordinates.push_back(NDC_center[0] - half_base);
    coordinates.push_back(NDC_center[1] + half_height + 0.008f);
    coordinates.push_back(0.0f);
    coordinates.push_back(1.0f);

    coordinates.push_back(NDC_center[0] + half_base);
    coordinates.push_back(NDC_center[1] + half_height + 0.008f);
    coordinates.push_back(0.0f);
    coordinates.push_back(1.0f);

    coordinates.push_back(point_x);
    coordinates.push_back(point_y);
    coordinates.push_back(0.0f);
    coordinates.push_back(1.0f);

    // Definir a cor azul para o dígito um
    for(GLuint i = 0; i < 5; i++){
        colors.push_back(0.0f);
        colors.push_back(0.0f);
        colors.push_back(1.0f);
        colors.push_back(1.0f);
    }

    // Gerar a topologia do dígito um
    topology.push_back(0);
    topology.push_back(1);
    topology.push_back(2);
    topology.push_back(3);
    topology.push_back(2);
    topology.push_back(1);
    topology.push_back(3);
    topology.push_back(2);
    topology.push_back(4);

    // Construir os VBOs para a posição geométrica

    glGenBuffers(1, &VBO_NDC_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, coordinates.size() * sizeof(GLfloat), coordinates.data(), GL_STATIC_DRAW);
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, point_coords, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir os VBOs para as informações de cores

    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, color_coding, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir o VBO para a topologia

    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, topology.size() * sizeof(GLuint), topology.data(), GL_STATIC_DRAW);

    GLuint size = 9;
    return size;
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Retorna o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula (slides 141-148 do documento Aula_03_Rendering_Pipeline_Grafico.pdf).
    glViewport(0, 0, width, height);
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ======================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// vim: set spell spelllang=pt_br :

