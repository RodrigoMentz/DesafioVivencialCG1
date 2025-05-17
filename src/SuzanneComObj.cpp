/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 13/08/2024
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

#include <cmath>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
std::string arquivoTextura;

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath, int &width, int &height);
int loadSimpleOBJ(string filePATH, int &nVertices);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

const GLchar* vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texc;
layout (location = 3) in vec3 normal;
uniform mat4 projection;
uniform mat4 model;
out vec2 texCoord;
out vec3 vertexColor;
void main()
{
	gl_Position = projection * model * vec4(position, 1.0);
	texCoord = vec2(texc.s, 1 - texc.t);
	vertexColor = color;
})";

const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
in vec3 vertexColor;
uniform sampler2D texBuff;
out vec4 color;

void main()
{
	vec4 texColor = texture(texBuff, texCoord);
	color = texColor;
})";

bool rotateX=false, rotateY=false, rotateZ=false;
float tamanhoEscala = 1.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
int objSelecionado = 1;

struct Object
{
	GLuint VAO; //Índice do buffer de geometria
	GLuint texID; //Identificador da textura carregada
	int nVertices; //nro de vértices
};

struct Material {
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    std::string textureFile;
};

std::unordered_map<std::string, Material> materiais;
std::string nomeMaterial;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio M3 - Rodrigo Korte Mentz", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	int imgWidth, imgHeight;
	Object obj;
	obj.VAO = loadSimpleOBJ("../assets/Modelos3D/SuzanneSubdiv1.obj", obj.nVertices);
	Object obj2;
	obj2.VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", obj.nVertices);
	
	Material mat = materiais["Material.001"];
    cout << "Conferir material.textureFile: " << mat.textureFile << endl;

	// Carregando uma textura e armazenando seu id
	obj.texID = loadTexture("../assets/Modelos3D/" + mat.textureFile,imgWidth,imgHeight);
	obj2.texID = loadTexture("../assets/Modelos3D/" + mat.textureFile,imgWidth,imgHeight);
	glUseProgram(shaderID);

	// Enviar a informação de qual variável armazenará o buffer da textura
	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

	//Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);
	

	// Matriz de projeção paralela ortográfica
	mat4 projection = ortho(-3.0, 3.0, -3.0, 3.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	// Matriz de modelo: transformações na geometria (objeto)
	mat4 model = mat4(1); // matriz identidade
	mat4 model2 = mat4(1);
	//glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	GLint modelLoc2 = glGetUniformLocation(shaderID, "model");
	
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model2 = glm::rotate(model2, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(model2));
	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1); 
		model2 = glm::mat4(1); 
		if (rotateX)
		{
			if (objSelecionado == 1)
			{
				model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			else
			{
				model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			}
		}
		else if (rotateY)
		{
			if (objSelecionado == 1)
			{
				model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
		else if (rotateZ)
		{
			if (objSelecionado == 1)
			{
				model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			}
			else
			{
				model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}

		// mover o obj
		model = glm::translate(model, glm::vec3(posX, posY, posZ));

		// mudar a escala do obj
		model = glm::scale(model, glm::vec3(tamanhoEscala, tamanhoEscala, tamanhoEscala));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(obj.VAO); // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, obj.texID); //conectando com o buffer de textura que será usado no draw
		glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
		
		glBindVertexArray(obj2.VAO); // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, obj2.texID); //conectando com o buffer de textura que será usado no draw
		glDrawArrays(GL_TRIANGLES, 0, obj2.nVertices);

		glBindVertexArray(0); // Desconectando o buffer de geometria

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &obj.VAO);
	glDeleteVertexArrays(1, &obj2.VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		objSelecionado = 1;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		objSelecionado = 2;
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS) 
	{
		posY += 0.1f;
	}

    if (key == GLFW_KEY_S && action == GLFW_PRESS) 
	{
		posY -= 0.1f;
	}

    if (key == GLFW_KEY_A && action == GLFW_PRESS) 
	{
		posX -= 0.1f;
	}

    if (key == GLFW_KEY_D && action == GLFW_PRESS) 
	{
		posX += 0.1f;
	}

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) 
	{
		posZ += 0.1f;
	}
	
    if (key == GLFW_KEY_E && action == GLFW_PRESS) 
	{
		posZ -= 0.1f;
	}

	if (key == GLFW_KEY_U && action == GLFW_PRESS)
	{
		tamanhoEscala += 0.1f;
	}

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		if (tamanhoEscala < 0.1f)
		{
			tamanhoEscala = 0.1f;
		}

		tamanhoEscala -= 0.1f;
	}
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

int loadSimpleOBJ(string filePATH, int &nVertices)
 {
    std::string nomeArquivoMtl;
    float ka, ks, ke;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    GLuint texturaId = 0;

    std::ifstream arqEntrada(filePATH.c_str());
    if (!arqEntrada.is_open()) 
	{
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(arqEntrada, line)) 
	{
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "mtllib") 
		{
            ssline >> nomeArquivoMtl;
        } 
        if (word == "v") 
		{
            glm::vec3 vertice;
            ssline >> vertice.x >> vertice.y >> vertice.z;
            vertices.push_back(vertice);
        } 
        else if (word == "vt") 
		{
            glm::vec2 vt;
            ssline >> vt.s >> vt.t;
            texCoords.push_back(vt);
        } 
        else if (word == "vn") 
		{
            glm::vec3 normal;
            ssline >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } 
        else if (word == "f")
		 {
            while (ssline >> word) 
			{
                int vi = 0, ti = 0, ni = 0;
                std::istringstream ss(word);
                std::string index;

                if (std::getline(ss, index, '/')) vi = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index, '/')) ti = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index)) ni = !index.empty() ? std::stoi(index) - 1 : 0;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);
                vBuffer.push_back(color.r);
                vBuffer.push_back(color.g);
                vBuffer.push_back(color.b);

                if (ti >= 0 && ti < texCoords.size()) {
                    vBuffer.push_back(texCoords[ti].x);
                    vBuffer.push_back(texCoords[ti].y);
                } else {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                }
                
                // Adicionando normais (nx, ny, nz)
                if (ni >= 0 && ni < normals.size()) {
                    vBuffer.push_back(normals[ni].x);
                    vBuffer.push_back(normals[ni].y);
                    vBuffer.push_back(normals[ni].z);
                } else {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                }
            }
        }
    }

    arqEntrada.close();

    if (!nomeArquivoMtl.empty()) {
        std::string diretorioObj = filePATH.substr(0, filePATH.find_last_of("/\\") + 1);
        std::string caminhoMTL = diretorioObj + nomeArquivoMtl;
        std::ifstream arqMTL(caminhoMTL.c_str());
        if (arqMTL.is_open()) {
            std::string mtlLine;
            while (std::getline(arqMTL, mtlLine)) {
                std::istringstream ssmtl(mtlLine);
                std::string mtlWord;
                ssmtl >> mtlWord;

                if (mtlWord == "newmtl") {
                    ssmtl >> nomeMaterial;
                    materiais[nomeMaterial] = Material();
                    cout << "Nome material: " << nomeMaterial << endl;
                }

                if (mtlWord == "map_Kd") {
                    ssmtl >> materiais[nomeMaterial].textureFile;
                    cout << "Nome arquivo textura: " << materiais[nomeMaterial].textureFile << endl;
                }
            }
            arqMTL.close();
        }
    }   

    std::cout << "Gerando o buffer de geometria..." << std::endl;
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

	GLsizei stride = 11 * sizeof(GLfloat);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);                    // posição
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat))); // cor
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(6 * sizeof(GLfloat))); // texcoord
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(8 * sizeof(GLfloat))); // normal

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	nVertices = vBuffer.size() / 11;  // x, y, z, r, g, b, s, t, nx, ny, nz (valores atualmente armazenados por vértice)
	cout << "nVertices: " << nVertices << endl;
    return VAO;
}