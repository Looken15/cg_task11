
// Градиентнйы треугольник

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;
// ID атрибута вершин
GLint Attrib_vertex;
// ID атрибута цвета
GLint Attrib_color;
// ID юниформ переменной цвета
GLint Unif_color;
// ID VBO вершин
GLuint VBO_position;
// ID VBO цвета
GLuint VBO_color;
// Вершина
struct Vertex
{
	GLfloat x;
	GLfloat y;
};

struct Color
{
	GLfloat R;
	GLfloat G;
	GLfloat B;
	GLfloat A;
};

GLint Unif_scale;
float scale[2] = { 1.0f, 1.0f };

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core
    in vec2 coord;
    in vec4 color;

	uniform vec2 scale;

    out vec4 vert_color;

    void main() {
        vert_color = color;
		vec3 pos = vec3(coord, 0.0);
		pos = pos * mat3(
						scale[0], 0, 0,
						0, scale[1], 0,
						0, 0, 1
						);		


        // Присваиваем вершину волшебной переменной gl_Position
        gl_Position = vec4(pos, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core
    in vec4 vert_color;

    out vec4 color;
    void main() {
        color = vert_color;
    }
)";


void Init();
void Draw();
void Release();


int main() {
	sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);

	window.setActive(true);

	// Инициализация glew
	glewInit();
	glEnable(GL_DEPTH_TEST);

	Init();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
			}

			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case (sf::Keyboard::A): scale[0] -= 0.1; break;
				case (sf::Keyboard::D): scale[0] += 0.1; break;
				case (sf::Keyboard::S): scale[1] -= 0.1; break;
				case (sf::Keyboard::W): scale[1] += 0.1; break;
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw();

		window.display();
	}

	Release();
	return 0;
}


// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror() {
	GLenum errCode;
	// Коды ошибок можно смотреть тут
	// https://www.khronos.org/opengl/wiki/OpenGL_Error
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}

// Функция печати лога шейдера
void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	char* infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}

Color HSVtoRGB(float H, float S, float V) {
	float s = S / 100;
	float v = V / 100;
	float C = s * v;
	float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	float m = v - C;
	float r, g, b;
	if (H >= 0 && H < 60) {
		r = C, g = X, b = 0;
	}
	else if (H >= 60 && H < 120) {
		r = X, g = C, b = 0;
	}
	else if (H >= 120 && H < 180) {
		r = 0, g = C, b = X;
	}
	else if (H >= 180 && H < 240) {
		r = 0, g = X, b = C;
	}
	else if (H >= 240 && H < 300) {
		r = X, g = 0, b = C;
	}
	else {
		r = C, g = 0, b = X;
	}
	float R = (r + m);
	float G = (g + m);
	float B = (b + m);

	return { R, G, B, 1.0 };
}

void InitVBO()
{
	glGenBuffers(1, &VBO_position);
	glGenBuffers(1, &VBO_color);

	/*std::vector<Vertex> triangle_vec = {
		{0.0, 0.0}, {0.0, 0.5},
		{0.0, 0.0}, {0.5, 0.0},
		{0.0, 0.0}, {0.0, -0.5},
		{0.0, 0.0}, {-0.5, 0.0},
	};*/

	const int tr = 361;

	std::vector<Vertex> triangle_vec;
	triangle_vec.push_back({ 0.0, 0.0 });
	for (int i = 0; i < tr; ++i)
	{
		float t = 2.0f * 3.1415f * i / 360;
		float x = 0.5f * std::cosf(t);
		float y = 0.5f * std::sinf(t);
		triangle_vec.push_back({ x, y });
	}

	Vertex triangle[2 * tr];
	std::copy(triangle_vec.begin(), triangle_vec.end(), triangle);

	std::vector<Color> colors_vec;
	colors_vec.push_back({ 1.0, 1.0, 1.0, 1.0 });
	for (int i = 0; i < tr; ++i)
	{
		colors_vec.push_back(HSVtoRGB(i, 100, 100));
	}
	Color colors[2 * tr];
	std::copy(colors_vec.begin(), colors_vec.end(), colors);

	// Передаем вершины в буфер
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	checkOpenGLerror();
}


void InitShader() {
	// Создаем вершинный шейдер
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// Передаем исходный код
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	ShaderLog(vShader);

	// Создаем фрагментный шейдер
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Передаем исходный код
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	ShaderLog(fShader);

	// Создаем программу и прикрепляем шейдеры к ней
	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);

	// Линкуем шейдерную программу
	glLinkProgram(Program);
	// Проверяем статус сборки
	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	// Вытягиваем ID атрибута вершин из собранной программы
	Attrib_vertex = glGetAttribLocation(Program, "coord");
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib coord" << std::endl;
		return;
	}

	// Вытягиваем ID атрибута цвета
	Attrib_color = glGetAttribLocation(Program, "color");
	if (Attrib_color == -1)
	{
		std::cout << "could not bind attrib color" << std::endl;
		return;
	}

	// Вытягиваем ID юниформ
	const char* unif_name = "scale";
	Unif_color = glGetUniformLocation(Program, unif_name);
	if (Unif_color == -1)
	{
		std::cout << "could not bind uniform " << unif_name << std::endl;
		return;
	}

	checkOpenGLerror();
}

void Init() {
	InitShader();
	InitVBO();
}


void Draw() {
	// Устанавливаем шейдерную программу текущей
	glUseProgram(Program);

	glUniform2fv(Unif_scale, 1, scale);

	// Включаем массивы атрибутов
	glEnableVertexAttribArray(Attrib_vertex);
	glEnableVertexAttribArray(Attrib_color);

	// Подключаем VBO_position
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glVertexAttribPointer(Attrib_vertex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Подключаем VBO_color
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glVertexAttribPointer(Attrib_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Отключаем VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Передаем данные на видеокарту(рисуем)
	glDrawArrays(GL_TRIANGLE_FAN, 0, 362);

	// Отключаем массивы атрибутов
	glDisableVertexAttribArray(Attrib_vertex);
	glDisableVertexAttribArray(Attrib_color);

	glUseProgram(0);
	checkOpenGLerror();
}


// Освобождение шейдеров
void ReleaseShader() {
	// Передавая ноль, мы отключаем шейдрную программу
	glUseProgram(0);
	// Удаляем шейдерную программу
	glDeleteProgram(Program);
}

// Освобождение буфера
void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_position);
	glDeleteBuffers(1, &VBO_color);
}

void Release() {
	ReleaseShader();
	ReleaseVBO();
}
