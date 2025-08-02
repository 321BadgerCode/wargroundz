#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <functional>

using namespace std;

typedef unsigned char u8;

// Window dimensions
const int WIDTH = 800, HEIGHT = 600;
int bufferWidth, bufferHeight;

enum XDirection { NONE, LEFT, RIGHT };

// Animation class
class Animation {
public:
	int current_frame;
	int last_time;
	float framerate;
	map<int, vector<GLuint>> textures;
	int current_state;

	Animation() {
		current_frame = 0;
		framerate = 1;
		current_state = 0;
	}

	void addState(int state, vector<GLuint> textures) {
		this->textures[state] = textures;
	}

	void setState(int state) {
		current_state = state;
	}

	void setFramerate(float framerate) {
		this->framerate = framerate;
	}

	void update() {
		int current_time = glfwGetTime() * 1000;
		int delta_time = current_time - last_time;

		if (delta_time > 1000 / framerate) {
			current_frame = (current_frame + 1) % textures[current_state].size();
			last_time = current_time;
		}
	}

	void draw(float x, float y, float width, float height) {
		if (!textures[current_state].empty()) {
			current_frame %= textures[current_state].size();
		} else {
			current_frame = 0;
		}

		glBindTexture(GL_TEXTURE_2D, textures[current_state][current_frame]);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1, 0);
		glVertex2f(x + width, y);
		glTexCoord2f(1, 1);
		glVertex2f(x + width, y + height);
		glTexCoord2f(0, 1);
		glVertex2f(x, y + height);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void draw(float x, float y, float width, float height, XDirection direction) {
		if (textures[current_state].empty()) {
			return;
		}

		current_frame %= textures[current_state].size();

		glBindTexture(GL_TEXTURE_2D, textures[current_state][current_frame]);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1, 0);
		glVertex2f(x + width * (direction == XDirection::RIGHT ? 1 : -1), y);
		glTexCoord2f(1, 1);
		glVertex2f(x + width * (direction == XDirection::RIGHT ? 1 : -1), y + height);
		glTexCoord2f(0, 1);
		glVertex2f(x, y + height);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
};

// Object class
class Object {
public:
	float x, y, width, height;
	GLuint texture;
	XDirection xDirection;

	Object() {}
	Object(float x, float y, float width, float height, GLuint texture) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->texture = texture;
	}

	void draw(GLuint texture) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1, 0);
		glVertex2f(x + width, y);
		glTexCoord2f(1, 1);
		glVertex2f(x + width, y + height);
		glTexCoord2f(0, 1);
		glVertex2f(x, y + height);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	void draw(GLuint texture, XDirection direction) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
		glTexCoord2f(1, 0);
		glVertex2f(x + width * (direction == XDirection::RIGHT ? 1 : -1), y);
		glTexCoord2f(1, 1);
		glVertex2f(x + width * (direction == XDirection::RIGHT ? 1 : -1), y + height);
		glTexCoord2f(0, 1);
		glVertex2f(x, y + height);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	void draw() {
		draw(texture);
	}

	void flip(XDirection direction = XDirection::NONE) {
		if (direction == XDirection::NONE) {
			if (xDirection == XDirection::LEFT) {
				xDirection = XDirection::RIGHT;
			} else if (xDirection == XDirection::RIGHT) {
				xDirection = XDirection::LEFT;
			}
		} else {
			xDirection = direction;
		}
	}
};

// Platform class
class Platform : public Object {
public:
	Platform(float x, float y, float width, float height, GLuint texture) : Object(x, y, width, height, texture) {}
};

// Checkpoint class
class Checkpoint : public Object {
public:
	GLuint textureReached;
	bool reached;

	Checkpoint() {
		reached = false;
	}
	Checkpoint(float x, float y, float width, float height, GLuint texture, GLuint textureReached) : Object(x, y, width, height, texture) {
		this->textureReached = textureReached;
		reached = false;
	}
};

// Gun class
class Gun : public Object {
public:
	float fireRate;
	float damage;

	Gun() {}
	Gun(float x, float y, float width, float height, GLuint texture, float fireRate, float damage) : Object(x, y, width, height, texture) {
		this->xDirection = XDirection::RIGHT;
		this->fireRate = fireRate;
		this->damage = damage;
	}

	// Object raycast(float distance) {
	// 	float xOffset = (xDirection == XDirection::RIGHT ? 1 : -1) * distance;
	// 	float yOffset = 0;

	// 	for (NPC npc : npcs) {
	// 		float rayX = x + xOffset;
	// 		float rayY = y + yOffset;

	// 		if (rayX > npc.x && rayX < npc.x + npc.width &&
	// 			rayY > npc.y && rayY < npc.y + npc.height) {
	// 			return npc;
	// 		}
	// 	}
	// 	return Object(0, 0, 0, 0, 0);
	// }
};

// Player class
class Player : public Object {
public:
	float speed;
	float dx, dy;
	float gravity;
	float jumpStrength;
	bool onGround;
	Checkpoint lastCheckpoint;
	Animation animation;
	Gun gun;

	Player(float x, float y, float width, float height, GLuint texture, float speed, float gravity, float jumpStrength) : Object(x, y, width, height, texture) {
		this->xDirection = XDirection::RIGHT;
		this->speed = speed;
		this->gravity = gravity;
		this->jumpStrength = jumpStrength;
		dx = 0;
		dy = 0;
		onGround = false;
	}

	void update() {
		animation.update();

		if (!onGround) {
			dy -= gravity;
		}

		y += dy;

		if (y < -1) {
			die();
		}

		gun.x = xDirection == XDirection::RIGHT ? x + (width / 2) : x - (width / 2);
		gun.y = y + height / 2;
	}

	void jump() {
		if (onGround) {
			dy = jumpStrength;
			onGround = false;
		}
	}

	void die() {
		x = lastCheckpoint.x;
		y = lastCheckpoint.y;
		dx = 0;
		dy = 0;
		onGround = false;
	}

	void changeGun(Gun gun) {
		this->gun = gun;
	}

	void draw() {
		gun.draw(gun.texture, xDirection);
		animation.draw(x, y, width, height, xDirection);
	}
};

// Player player(-1, 1, .2, .4, 0, .01, .001, .05);
Player player(-1, 1, .2, .4, 0, .015, .0025, .08);

// Dialogue choice class
class DialogueChoice {
public:
	string text;
	function<void()> action;

	DialogueChoice(string text, function<void()> action) {
		this->text = text;
		this->action = action;
	}
};

// Dialogue node class
class DialogueNode {
public:
	string text;
	vector<DialogueChoice> choices;

	DialogueNode() {}
	DialogueNode(string text) {
		this->text = text;
	}

	void addChoice(DialogueChoice choice) {
		choices.push_back(choice);
	}
};

// NPC class
class NPC : public Object {
public:
	float speed;
	float dx, dy;
	float gravity;
	bool onGround;
	Animation animation;
	map<string, DialogueNode> dialogue;

	NPC(float x, float y, float width, float height, GLuint texture, float speed, float gravity) : Object(x, y, width, height, texture) {
		this->speed = speed;
		dx = 0;
		dy = 0;
		onGround = false;
	}

	void update() {
		animation.update();

		if (!onGround) {
			dy -= gravity;
		}

		y += dy;
	}

	void draw() {
		xDirection = player.x > x ? XDirection::RIGHT : XDirection::LEFT;
		animation.draw(x, y, width, height, xDirection);
	}
};

// Camera class
struct Camera {
	float x, y, speed, distance_threshold, zoom;

	void update(Player player) {
		// Camera target position
		float targetX = player.x - distance_threshold;

		// Follow forward and backward with easing using speed and interpolation
		x += (targetX - x / zoom) * speed;

		if (x < 0) {
			x = 0;
		}

		// 	// Zoom in and out based on player's y position
		// 	zoom = 1 + player.y * .1;
		// 	y = player.y;
	}
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// Update the OpenGL viewport to match the window size
	glViewport(0, 0, width, height);

	// Set up an orthographic projection matrix that scales with window size
	float aspectRatio = (float)width / (float)height;
	float left = -aspectRatio;
	float right = aspectRatio;
	float bottom = -1;
	float top = 1;

	// Set up the orthographic projection (scalable with the window size)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Load a texture from a file
GLuint loadTexture(const char* filename) {
	GLuint texture = SOIL_load_OGL_texture(
		filename, 
		SOIL_LOAD_AUTO, 
		SOIL_CREATE_NEW_ID, 
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
	);

	if (texture == 0) {
		std::cerr << "Error loading texture: " << filename << std::endl;
		std::cerr << SOIL_last_result() << std::endl;
	}

	return texture;
}

// Texture ID for font atlas
GLuint fontTexture;

// Load a texture containing a character map
void loadFontTexture(const char* filePath) {
	fontTexture = SOIL_load_OGL_texture(filePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
}

// Render a single character from the font atlas
// TODO: 'o' has a bit of '_' (the character above 'o' in the font atlas) at the top hovering over it when rendered
void renderCharacter(char c, float x, float y, float size) {
	// The width and height of each character in the font texture (16x16)
	const int CHAR_WIDTH = 16;
	const int CHAR_HEIGHT = 16;

	// Total width and height of the font texture
	const int TEXTURE_WIDTH = 256; // Example: 16x16 characters in a 256x256 texture
	const int TEXTURE_HEIGHT = 256; // Example: 16x16 characters in a 256x256 texture

	// Calculate the row and column in the texture for the character
	int c_normalized = c - ' ';
	int column = (c_normalized % 16); // Column of the character in the texture (e.g., A=0, B=1, ...)
	int row = (c_normalized / 16); // Row of the character in the texture (e.g., A=0, B=0, 0=1, 1=1, ...)

	// Calculate the texture coordinates for the character
	float u = column * (CHAR_WIDTH / (float)TEXTURE_WIDTH);
	float v = row * (CHAR_HEIGHT / (float)TEXTURE_HEIGHT);
	float u2 = (column + 1) * (CHAR_WIDTH / (float)TEXTURE_WIDTH);
	float v2 = (row + 1) * (CHAR_HEIGHT / (float)TEXTURE_HEIGHT);

	// Draw the character as a textured quad
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
		glTexCoord2f(u, v2); glVertex2f(x, y);
		glTexCoord2f(u2, v2); glVertex2f(x + size, y);
		glTexCoord2f(u2, v); glVertex2f(x + size, y + size);
		glTexCoord2f(u, v); glVertex2f(x, y + size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

// Render a string of text
void renderText(const char* text, float x, float y, float size) {
	float offset = 0;
	while (*text) {
		if (*text == '\n') {
			y -= size * 1.2; // Adjust line spacing
			offset = 0;
		} else {
			renderCharacter(*text, x + offset, y, size);
			offset += size * .6; // Adjust character spacing
		}
		text++;
	}
}

Gun gun(0, 0, .1, .1, 0, .1, 1);
NPC billy(1, 1, .2, .4, 0, .015, .0025);
vector<NPC> npcs;
Camera camera = {0, 0, .1, .01, .5};
// Camera camera = {0, 0, .1, .01, 1};
vector<Platform> platforms = {
	Platform(-2, -1, 4, .2, 0),
	Platform(-1, 0, 2, .2, 0),
	Platform(2.5, -1, 4, .2, 0)
};
vector<Checkpoint> checkpoints = {
	Checkpoint(-1, 1, .2, .2, 0, 0),
	Checkpoint(1.5, .2, .2, .2, 0, 0)
};

void handleCollisions();

int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		cout << "GLFW initialization failed!" << endl;
		glfwTerminate();
		return 1;
	}

	// Create the window
	GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);
		if (!mainWindow) {
		cout << "GLFW window creation failed!" << endl;
		glfwTerminate();
		return 1;
	}

	// Register the resize callback function
	glfwSetFramebufferSizeCallback(mainWindow, framebuffer_size_callback);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Get buffer size information
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Allow modern extension features
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		cout << "GLEW initialization failed!" << endl;
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	framebuffer_size_callback(mainWindow, WIDTH, HEIGHT);

	// Disable depth test
	glDisable(GL_DEPTH_TEST);

	// Set game loop update rate to be 75% slower than normal refresh rate
	glfwSwapInterval(1.75);

	// Set clear color
	glClearColor(.2, .2, .2, 1);

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load font texture
	loadFontTexture("./font_atlas.png");

	// Load textures
	gun.texture = loadTexture("./gun.png");
	player.texture = loadTexture("./player.png");
	billy.texture = loadTexture("./player.png");
	for (Platform& platform : platforms) {
		platform.texture = loadTexture("./platform.png");
	}
	for (Checkpoint& checkpoint : checkpoints) {
		checkpoint.texture = loadTexture("./checkpoint.png");
		checkpoint.textureReached = loadTexture("./checkpoint_reached.png");
	}

	// Initialize scene
	player.lastCheckpoint = checkpoints[0];
	player.gun = gun;
	player.animation.addState(0, {loadTexture("./player.png"), loadTexture("./player2.png")});

	// Create Billy NPC
	billy.animation.addState(0, {loadTexture("./player.png"), loadTexture("./player2.png")});
	billy.dialogue["hello"] = DialogueNode("Hello! How are you?");
	billy.dialogue["hello"].addChoice(DialogueChoice("I'm good, thanks!", []() {
		cout << "I'm good, thanks!" << endl;
	}));
	billy.dialogue["hello"].addChoice(DialogueChoice("I'm not doing well.", []() {
		cout << "I'm not doing well." << endl;
	}));
	npcs.push_back(billy);

	// Loop until window closed
	while (!glfwWindowShouldClose(mainWindow)) {
		// Get & handle user input events
		glfwPollEvents();

		// Clear window
		glClear(GL_COLOR_BUFFER_BIT);

		// Exit game if ESC is pressed
		if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(mainWindow, GL_TRUE);
		}

		// Update camera
		camera.update(player);

		// Update display based on camera position & zoom
		glLoadIdentity();
		glTranslatef(-camera.x, -camera.y, 0);
		glScalef(camera.zoom, camera.zoom, 1);

		// Display greeting text
		renderText("...You thought you could escape?\nThat's what they all think.", -.5, .8, .1);

		// Keyboard input for arrow keys
		if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS && player.x > -1) {
			player.x -= player.speed;
			player.flip(XDirection::LEFT);
		} if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			player.x += player.speed;
			player.flip(XDirection::RIGHT);
		} if (glfwGetKey(mainWindow, GLFW_KEY_UP) == GLFW_PRESS && player.onGround) {
			player.jump();
		}
		if (glfwGetKey(mainWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
			Object hitObject = raycast(player, 1, player.xDirection);
			cout << "Hit object: " << hitObject.x << ", " << hitObject.y << endl;
		}

		// Update objects
		player.update();
		for (NPC & npc : npcs) {
			npc.update();
		}

		// Handle collision
		handleCollisions();

		// Draw objects
		player.draw();
		for (Platform platform : platforms) {
			platform.draw();
		}
		for (Checkpoint checkpoint : checkpoints) {
			if (checkpoint.reached) {
				checkpoint.draw(checkpoint.textureReached);
			} else {
				checkpoint.draw();
			}
		}
		for (NPC npc : npcs) {
			npc.draw();
		}

		// Swap buffers
		glfwSwapBuffers(glfwGetCurrentContext());
	}

	// Clean up memory
	glDeleteTextures(1, &fontTexture);
	for (Platform platform : platforms) {
		glDeleteTextures(1, &platform.texture);
	}
	glDeleteTextures(1, &player.texture);

	// Clean up
	glfwDestroyWindow(mainWindow);
	glfwTerminate();

	return 0;
}

enum class Direction { TOP = 0b0001, BOTTOM = 0b0010, LEFT = 0b0100, RIGHT = 0b1000 };
u8 getCollisionDirection(const Object& player, const Object& object) {
	u8 direction = 0;

	// Player's boundaries
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float playerTop = player.y + player.height;
	float playerBottom = player.y;

	// Object's boundaries
	float objectLeft = object.x;
	float objectRight = object.x + object.width;
	float objectTop = object.y + object.height;
	float objectBottom = object.y;

	// Check for collision
	if (playerBottom < objectTop && playerTop > objectBottom) {
		if (playerRight > objectLeft && playerLeft < objectRight) {
			float overlapX = min(playerRight, objectRight) - max(playerLeft, objectLeft);
			float overlapY = min(playerTop, objectTop) - max(playerBottom, objectBottom);

			if (overlapX > overlapY) {
				if (playerBottom < objectBottom) {
					direction |= static_cast<u8>(Direction::BOTTOM);
				} else {
					direction |= static_cast<u8>(Direction::TOP);
				}
			} else {
				if (playerLeft < objectLeft) {
					direction |= static_cast<u8>(Direction::LEFT);
				} else {
					direction |= static_cast<u8>(Direction::RIGHT);
				}
			}
		}
	}

	return direction;
}

void handleCollisions() {
	// Platform collision
	bool isOnPlatform = false;

	for (Platform platform : platforms) {
		u8 direction = getCollisionDirection(player, platform);

		// Check if player is on the ground
		if (direction & static_cast<u8>(Direction::TOP) && player.dy <= 0) {
			player.dy = 0;
			player.y = platform.y + platform.height;
			player.onGround = true;
			isOnPlatform = true;
		} if (direction & static_cast<u8>(Direction::BOTTOM)) {
			float damping = .5;
			player.dy = -player.dy * damping;
		}

		// Handle left and right collisions
		if (direction & static_cast<u8>(Direction::LEFT)) {
			player.x = platform.x - player.width;
		} if (direction & static_cast<u8>(Direction::RIGHT)) {
			player.x = platform.x + platform.width;
		}
	}

	if (!isOnPlatform) {
		player.onGround = false;
	}

	// Checkpoint collision
	for (Checkpoint& checkpoint : checkpoints) {
		u8 direction = getCollisionDirection(player, checkpoint);

		if (direction != 0b0000 && !checkpoint.reached) {
			checkpoint.reached = true;
			player.lastCheckpoint = checkpoint;
		}
	}

	// NPC collision
	for (NPC npc : npcs) {
		u8 direction = getCollisionDirection(player, npc);

		if (direction != 0b0000) {
			float textWidth = 0;
			for (char c : npc.dialogue["hello"].text) {
				textWidth += .04;
			}
			renderText(npc.dialogue["hello"].text.c_str(), npc.x - (textWidth / 2), npc.y + npc.height + .05, .1);
		}
	}
}