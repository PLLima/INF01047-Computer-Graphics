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

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
GLuint BuildScene(GLuint count);
std::vector<GLubyte> DecimalToBinary(GLuint decimal_number, GLuint range);
GLubyte BuildZero(std::vector<GLfloat> *coordinates, std::vector<GLfloat> *colors, std::vector<GLubyte> *topology, GLubyte last_point, std::vector<GLfloat> NDC_center, GLuint external_points_count);
GLubyte BuildOne(std::vector<GLfloat> *coordinates, std::vector<GLfloat> *colors, std::vector<GLubyte> *topology, GLubyte last_point, std::vector<GLfloat> NDC_center);
GLuint BuildTriangles(GLfloat external_radius, GLfloat internal_radius, GLuint external_points_count); // Constrói triângulos para renderização
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

    // Construímos a representação de um triângulo
    GLfloat external_radius = 0.7f;
    GLfloat internal_radius = 0.5f;
    GLuint external_points_count = 16;
    GLuint vertex_array_object_id = BuildTriangles(external_radius, internal_radius, external_points_count);

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima
        glClear(GL_COLOR_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos)
        glUseProgram(g_GpuProgramID);

        // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
        // vértices apontados pelo VAO criado pela função BuildTriangles(). Veja
        // comentários detalhados dentro da definição de BuildTriangles().
        glBindVertexArray(vertex_array_object_id);

        // Pedimos para a GPU rasterizar os vértices apontados pelo VAO como
        // triângulos
        glDrawElements(GL_TRIANGLE_STRIP, 2 * external_points_count + 2, GL_UNSIGNED_BYTE, 0);

        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs
        glBindVertexArray(0);

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
GLuint BuildScene(GLuint count){
    
    // Definir dados vetoriais
    GLuint point_coords = 4;
    GLuint color_coding = 4;

    // Alocar as coordenadas dos pontos
    std::vector<GLfloat> NDC_coefficients;

    // Alocar as cores dos pontos
    std::vector<GLfloat> color_coefficients;

    // Alocar o vetor de índices
    std::vector<GLubyte> indices;

    // Converter decimal para binário (little endian)
    std::vector<GLubyte> binary_count = DecimalToBinary(count, 4);

    for(GLuint i : binary_count){
        
    }

    // Construir os VBOs para a posição geométrica
    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, NDC_coefficients.size() * sizeof(GLfloat), NDC_coefficients.data(), GL_STATIC_DRAW);
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, point_coords, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir os VBOs para as informações de cores
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, color_coefficients.size() * sizeof(GLfloat), color_coefficients.data(), GL_STATIC_DRAW);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, color_coding, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir o VBO para a topologia
    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLubyte), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    return vertex_array_object_id;
}

// Converter decimal para um vetor binário (little endian)
std::vector<GLubyte> DecimalToBinary(GLuint decimal_number, GLuint range){
    std::vector<GLubyte> binary;

    for(GLuint i = 0; i < range; i++){
        binary.push_back(decimal_number % 2);
        decimal_number = decimal_number / 2;
    }

    return binary;
}

// Gerar pontos, cores e topologia do dígito zero
GLubyte BuildZero(std::vector<GLfloat> *coordinates, std::vector<GLfloat> *colors, std::vector<GLubyte> *topology, GLubyte last_point, std::vector<GLfloat> NDC_center, GLuint external_points_count){
    
    
    GLubyte next_point = 2 * external_points_count + last_point;
    return next_point;
}

// Gerar pontos, cores e topologia do dígito um
GLubyte BuildOne(std::vector<GLfloat> *coordinates, std::vector<GLfloat> *colors, std::vector<GLubyte> *topology, GLubyte last_point, std::vector<GLfloat> NDC_center){
    // Definir tamanhos básicos do dígito
    GLfloat half_base = 0.025f;
    GLfloat half_height = 0.7f;
    GLfloat point_x = -0.1f + NDC_center[0];
    GLfloat point_y = 0.408f + NDC_center[1];
    
    // Definir coordenadas dos pontos
    coordinates->push_back(NDC_center[0] - half_base);
    coordinates->push_back(NDC_center[1] - half_height + 0.008f);
    coordinates->push_back(0.0f);
    coordinates->push_back(1.0f);

    coordinates->push_back(NDC_center[0] + half_base);
    coordinates->push_back(NDC_center[1] - half_height + 0.008f);
    coordinates->push_back(0.0f);
    coordinates->push_back(1.0f);

    coordinates->push_back(NDC_center[0] + half_base);
    coordinates->push_back(NDC_center[1] + half_height + 0.008f);
    coordinates->push_back(0.0f);
    coordinates->push_back(1.0f);

    coordinates->push_back(NDC_center[0] - half_base);
    coordinates->push_back(NDC_center[1] + half_height + 0.008f);
    coordinates->push_back(0.0f);
    coordinates->push_back(1.0f);

    coordinates->push_back(point_x);
    coordinates->push_back(point_y);
    coordinates->push_back(0.0f);
    coordinates->push_back(1.0f);

    // Definir a cor azul para o dígito um e os índices
    for(GLuint i = 0; i < 5; i++){
        colors->push_back(0.0f);
        colors->push_back(0.0f);
        colors->push_back(1.0f);
        colors->push_back(1.0f);

        topology->push_back(i + last_point);
    }

    GLubyte next_point = 5 + last_point;
    return next_point;
}

// Construir triângulos para futura renderização
GLuint BuildTriangles(GLfloat external_radius, GLfloat internal_radius, GLuint external_points_count)
{
    // Definir dados vetoriais
    GLuint point_coords = 4;
    GLuint color_coding = 4;

    // Alocar as coordenadas dos pontos
    std::vector<GLfloat> NDC_coefficients;

    // Alocar as cores dos pontos
    std::vector<GLfloat> color_coefficients;

    // Alocar o vetor de índices
    std::vector<GLubyte> indices;

    // Calcular e colocar todos os dados dos pontos (posição, cor e topologia)
    GLfloat step = 2.0f * M_PI / external_points_count;
    for(GLuint i = 0; i < external_points_count; i++){
        // Calcular os pontos internos
        NDC_coefficients.push_back(internal_radius * cosf(step * i));
        NDC_coefficients.push_back(internal_radius * sinf(step * i));
        NDC_coefficients.push_back(0.0f);
        NDC_coefficients.push_back(1.0f);
        // Calcular os pontos externos
        NDC_coefficients.push_back(external_radius * cosf(step * i));
        NDC_coefficients.push_back(external_radius * sinf(step * i));
        NDC_coefficients.push_back(0.0f);
        NDC_coefficients.push_back(1.0f);

        // Estabelecer a cor vermelha em todos os pontos internos
        color_coefficients.push_back(1.0f);
        color_coefficients.push_back(0.0f);
        color_coefficients.push_back(0.0f);
        color_coefficients.push_back(1.0f);
        // Estabelecer a cor azul em todos os pontos externos
        color_coefficients.push_back(0.0f);
        color_coefficients.push_back(0.0f);
        color_coefficients.push_back(1.0f);
        color_coefficients.push_back(1.0f);

        // Construir topologia de TRIANGLE_STRIP
        indices.push_back(GLubyte(2 * i));
        indices.push_back(GLubyte(2 * i + 1));
    }
    indices.push_back(0);
    indices.push_back(1);

    // Construir os VBOs para a posição geométrica
    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, NDC_coefficients.size() * sizeof(GLfloat), NDC_coefficients.data(), GL_STATIC_DRAW);
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, point_coords, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir os VBOs para as informações de cores
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, color_coefficients.size() * sizeof(GLfloat), color_coefficients.data(), GL_STATIC_DRAW);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    glVertexAttribPointer(location, color_coding, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construir o VBO para a topologia
    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLubyte), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    return vertex_array_object_id;
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

