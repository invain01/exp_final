
#include "stdafx.h"
#include "math.h"
#include "CVector.h"
#include "CMatrix.h"
#include <ctime>
#include <vector>
#include <atlimage.h>
#include <algorithm> 
#include "CEuler.h"
#include "glew.h"
#include "glut.h"
#include <atlconv.h>

GLuint texture[1] = { 0 };
bool keyState[256] = { false }; // 初始化所有键的状态为未按下
bool isAstronautView = false; // 当前是否为太空人视角
// 视点位置和方向
float mx = 0, my = 5, mz = 10, rx = -0.25, ry = 0, rz = 0; // 平移和旋转
float godView_mx = 0, godView_my = 5, godView_mz = 10;
float godView_rx = -0.25, godView_ry = 0, godView_rz = 0;
float sx = 1, sy = 1, sz = 1; // 缩放
float mspeed = 0.003, rspeed = 0.02;
float g_IEyeMat[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, g_EyeMat[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
int mode = 1; // 0: 矩阵模式，1: 直接模式
bool isInterpolating = false; // 是否正在进行插值
int interpolationFrame = 0; // 当前插值帧数
const int interpolationTotalFrames = 300; // 总插值帧数
CVector initialPosition, targetPosition; // 初始和目标视点位置
float initialRx, initialRy, initialRz; // 初始旋转角度
float targetRx, targetRy, targetRz; // 目标旋转角度
CQuaternion initialQuat, targetQuat; // 初始和目标四元数
CQuaternion currentQuat;            // 当前四元数

template <typename T>
T clamp(T value, T min, T max) {
	if (value < min) {
		return min;
	}
	else if (value > max) {
		return max;
	}
	else {
		return value;
	}
}

struct Astronaut {
	CVector position; // 太空人位置
	float angle; // 太空人的旋转角度
	float step; // 太空人的行走步数
};
Astronaut astronaut = { CVector(0, 0, 0), 0, 0 };
CVector v; // 球的速度向量
CVector stars[100]; // 星星的位置
CVector points[30][60];
CVector pos;//球的位置
float starBrightness[100]; // 星星的亮度
float t = 0; // 时间变量，用于控制星星亮度变化
void myDisplay(void);
void DrawStars();
void myKeyboard();
void DrawPlanets();
void DrawShip();
void DrawText(const char* text, float x, float y, float z, void* font);
void rotateConeToDirection(float vx, float vy, float vz);
float seta = 0;//自转速度
float sp = 0.02;//速度大小
struct Planet {
	CVector position;       // 行星位置
	float radius;           // 行星半径
	float orbitRadius;      // 公转轨道半径
	float orbitSpeed;       // 公转速度
	float rotationSpeed;    // 自转速度
	float angle;            // 当前角度
	CVector color;          // 行星颜色（备用）
	GLuint textureID;       // 行星纹理 ID（新增）
};

struct Spaceship {
    CVector position; // 飞船位置
    float angleX, angleY; // 飞船的旋转角度
    float speed; // 飞船速度
    CVector direction; // 飞船的移动方向
    int targetPlanet; // 目标行星的索引
};

Spaceship spaceship = { CVector(0, 0, 0), 0, 0, 0.02, CVector(0, 0, 0), -1 };

// 全局变量
std::vector<Planet> planets = {
	{CVector(0, 0, 0), 0.5 * 5, 0, 0, 0, 0, CVector(1.0, 1.0, 0.0),LoadGLTextureFromFile("8k_sun.jpg")}, // 太阳（黄色）
	{CVector(0, 0, 0), 0.2 * 5, 1.5 * 5, 0.01, 0.02, 0, CVector(0.8, 0.8, 0.8),LoadGLTextureFromFile("8k_mercury.jpg")}, // 水星（灰色）
	{CVector(0, 0, 0), 0.3 * 5, 2.0 * 5, 0.008, 0.015, 0, CVector(0.7, 0.6, 0.2),LoadGLTextureFromFile("8k_venus_surface.jpg")}, // 金星（黄褐色）
	{CVector(0, 0, 0), 0.4 * 5, 2.5 * 5, 0.006, 0.01, 0, CVector(0.2, 0.2, 0.9),LoadGLTextureFromFile("8k_earth_daymap.jpg")}, // 地球（蓝色）
	{CVector(0, 0, 0), 0.1 * 5, 2.6 * 5, 0.007, 0.012, 0, CVector(0.8, 0.8, 0.8),LoadGLTextureFromFile("8k_moon.jpg")}, // 月球（灰色）
	{CVector(0, 0, 0), 0.35 * 5, 3.0 * 5, 0.005, 0.008, 0, CVector(0.9, 0.2, 0.2),LoadGLTextureFromFile("8k_mars.jpg")}, // 火星（红色）
	{CVector(0, 0, 0), 0.6 * 5, 4.0 * 5, 0.004, 0.006, 0, CVector(0.9, 0.9, 0.2),LoadGLTextureFromFile("8k_jupiter.jpg")}, // 木星（浅黄色）
	{CVector(0, 0, 0), 0.55 * 5, 5.0 * 5, 0.003, 0.005, 0, CVector(0.8, 0.6, 0.2),LoadGLTextureFromFile("8k_saturn.jpg")}, // 土星（黄褐色）
};
int selectedPlanet = -1; // 当前选中的行星
bool wireframeMode = false; // 是否为线框模式
void myTimerFunc(int val)
{
    t += 0.033f;
    seta += 0.1f;
    for (size_t i = 1; i < planets.size(); ++i) {
        planets[i].angle += planets[i].orbitSpeed;
    }
    if (spaceship.targetPlanet != -1) {
        // 计算飞船到目标行星的距离
        float distance = (planets[spaceship.targetPlanet].position - spaceship.position).len();
        if (distance > spaceship.speed) {
            // 沿方向向量移动飞船
            spaceship.position = spaceship.position + spaceship.direction * spaceship.speed;
        }
        else {
            // 如果距离小于速度，直接到达目标位置
            spaceship.position = planets[spaceship.targetPlanet].position;
            spaceship.targetPlanet = -1; // 重置目标行星
        }
    }
    else {
        // 如果没有目标行星，按当前方向移动
        CVector direction = CVector(
            sin(spaceship.angleY * 3.1415 / 180.0),
            0,
            cos(spaceship.angleY * 3.1415 / 180.0)
        );
        direction.Normalize();
        spaceship.position = spaceship.position + direction * spaceship.speed;
    }
	// 更新宇航员视图下的视点位置
	// 更新宇航员视图下的视点位置
	if (isAstronautView) {
		mx = spaceship.position.x; // 摄像头位于飞船内部
		my = spaceship.position.y + 0.5f; // 视点稍微高于飞船底部
		mz = spaceship.position.z - 1.0f; // 视点位于飞船后方一定距离
		ry = spaceship.angleY; // 视点方向与飞船方向一致
	}
    myDisplay();
    glutTimerFunc(33, myTimerFunc, 0);
}

void SetRC()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#define A2R(x) (x/180.0*3.14159)

void DrawText(const char* text, float x, float y, float z, void* font) {
    glRasterPos3f(x, y, z);
    for (const char* p = text; *p; ++p) {
        glutBitmapCharacter(font, *p);
    }
}
bool rayIntersectsSphere(const CVector& rayOrigin, const CVector& rayDirection, const CVector& sphereCenter, float sphereRadius, float& t) {
    CVector L = sphereCenter - rayOrigin;
    float tca = -L.dotMul(rayDirection);
    //printf("tca=%f\n", tca);
    if (tca < 0) return false; // 射线方向与球心方向相反，无交点

    float d2 = L.dotMul(L) - tca * tca;
    float r2 = sphereRadius * sphereRadius;
    //printf("d2=%f,r2=%f\n",d2, r2);
    if (d2 > r2) return false; // 射线与球体不相交

    float thc = sqrt(r2 - d2);
    t = tca - thc; // 最近的交点
    return true;
}
void SetView() {
	if (isAstronautView) {
		// 太空人视角
		glTranslatef(-mx, -my, -mz);
		glRotatef(-rz, 0, 0, 1);
		glRotatef(-rx, 1, 0, 0);
		glRotatef(-ry, 0, 1, 0);
	}
	else {
		// 上帝视角
		if (mode == 0) {
			glLoadMatrixf(g_EyeMat);
		}
		else {
			glRotatef(-rz, 0, 0, 1);
			glRotatef(-rx, 1, 0, 0);
			glRotatef(-ry, 0, 1, 0);
			glTranslatef(-mx, -my, -mz);
		}
	}
}
void glutSolidCylinder(float radius, float height, int slices, int stacks) {
	float step = 2 * PI / slices; // 每个切片的角度
	float halfHeight = height / 2.0f; // 圆柱体的一半高度

	// 绘制圆柱体的侧面
	for (int i = 0; i < slices; ++i) {
		glBegin(GL_QUADS);
		{
			// 上圆
			glVertex3f(radius * cos(i * step), halfHeight, radius * sin(i * step));
			glVertex3f(radius * cos((i + 1) * step), halfHeight, radius * sin((i + 1) * step));
			// 下圆
			glVertex3f(radius * cos((i + 1) * step), -halfHeight, radius * sin((i + 1) * step));
			glVertex3f(radius * cos(i * step), -halfHeight, radius * sin(i * step));
		}
		glEnd();
	}

	// 绘制圆柱体的顶部
	glBegin(GL_POLYGON);
	for (int i = 0; i < slices; ++i) {
		glVertex3f(radius * cos(i * step), halfHeight, radius * sin(i * step));
	}
	glEnd();

	// 绘制圆柱体的底部
	glBegin(GL_POLYGON);
	for (int i = 0; i < slices; ++i) {
		glVertex3f(radius * cos(i * step), -halfHeight, radius * sin(i * step));
	}
	glEnd();
}
void mouse(int button, int state, int x, int y) {
    /*if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 获取鼠标点击位置的屏幕坐标
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        int winWidth = viewport[2];
        int winHeight = viewport[3];
        // 将鼠标坐标转换为 NDC 坐标
        float ndcX = (2.0f * x) / winWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * y) / winHeight;
        // 从深度缓冲区读取 Z 值
        float z, h;
        glReadPixels(x, winHeight - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
        glReadPixels(7.0f, winHeight - 7.0f, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &h);
        // 将 NDC 坐标转换为世界坐标
        CVector rayOrigin(7 * 2 / winWidth - 1.0f, 1 - (2.0f * 7) / winHeight, h); // 摄像机位置
        CVector rayDirection(ndcX, ndcY, z);
        rayDirection.Normalize();

        // 检测射线与每个行星的交点
        float closestT = std::numeric_limits<float>::max();
        int closestPlanet = -1;
        for (size_t i = 0; i < planets.size(); ++i) {
            float t;
            if (rayIntersectsSphere(rayOrigin, rayDirection, planets[i].position, planets[i].radius, t)) {
                if (t < closestT) {
                    closestT = t;
                    closestPlanet = i;
                }
            }
        }

        // 更新选中的行星
        selectedPlanet = closestPlanet;
    }
    glutPostRedisplay();*/
}
int LoadGLTextureFromFile(const char* filepath)
{
	CImage img;
	HRESULT hResult = img.Load(filepath);
	if (FAILED(hResult))
	{
		printf("Failed to load image: %s\n", filepath);
		return 0;
	}

	glGenTextures(1, &texture[0]);            // 创建纹理
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	int pitch = img.GetPitch();
	if (pitch < 0)
		gluBuild2DMipmaps(GL_TEXTURE_2D, img.GetBPP() / 8, img.GetWidth(), img.GetHeight(), GL_BGR, GL_UNSIGNED_BYTE, img.GetPixelAddress(0, img.GetHeight() - 1));
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, img.GetBPP() / 8, img.GetWidth(), img.GetHeight(), GL_BGR, GL_UNSIGNED_BYTE, img.GetBits());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	float col[4] = { 1, 1, 1, 1 };
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);

	return 1;  // 加载成功
}


void DrawAstronaut() {
	glPushMatrix();
	glTranslatef(spaceship.position.x, spaceship.position.y, spaceship.position.z); // 随飞船移动
	glTranslatef(astronaut.position.x, astronaut.position.y, astronaut.position.z); // 相对位置

	// 头部（黄色）
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // 黄色
	glTranslatef(0, 0.5, 0);
	glutSolidSphere(0.2, 20, 20);
	glPopMatrix();

	// 身体（红色）
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0); // 红色
	glTranslatef(0, 0, 0);
	glutSolidCube(0.5);
	glPopMatrix();

	// 左腿（蓝色）
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0); // 蓝色
	glTranslatef(-0.1, -0.25, 0);
	glRotatef(astronaut.step, 1, 0, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// 右腿（蓝色）
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0); // 蓝色
	glTranslatef(0.1, -0.25, 0);
	glRotatef(-astronaut.step, 1, 0, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// 左臂（绿色）
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // 绿色
	glTranslatef(-0.25, 0.25, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// 右臂（绿色）
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // 绿色
	glTranslatef(0.25, 0.25, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	glPopMatrix();
}
void DrawShip() {
	glPushMatrix();
	glTranslatef(spaceship.position.x, spaceship.position.y, spaceship.position.z);
	glRotatef(spaceship.angleX, 1, 0, 0);
	glRotatef(spaceship.angleY, 0, 1, 0);

	// 飞船头部
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0); // 红色
	glTranslatef(0, 0.5 * 5, 0);
	glutSolidSphere(0.3 * 5, 20, 20);
	glPopMatrix();

	// 飞船机舱
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // 绿色
	glTranslatef(0, 0, 0);
	glutSolidCube(1.5 * 5); // 机舱尺寸
	glPopMatrix();

	// 左侧机翼（向左延伸）
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // 黄色
	// 根部位于机舱左侧边缘外（机舱宽度1.5*5，中心在0，左侧边缘-0.75*5，向外延伸0.1）
	glTranslatef(-0.75 * 5 - 0.1, 0, 0);

	glBegin(GL_TRIANGLES);
	{
		// 顶部顶点：根部位置上方
		glVertex3f(0, 0.25 * 5, 0);
		// 底部顶点：根部位置下方
		glVertex3f(0, -0.25 * 5, 0);
		// 尖端顶点：向左延伸1.5*5（确保在机舱外部）
		glVertex3f(-1.5 * 5, 0, 0);
	}
	glEnd();
	glPopMatrix();

	// 右侧机翼（向右延伸）
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // 黄色
	// 根部位于机舱右侧边缘外（右侧边缘0.75*5，向外延伸0.1）
	glTranslatef(0.75 * 5 + 0.1, 0, 0);

	glBegin(GL_TRIANGLES);
	{
		// 顶部顶点：根部位置上方
		glVertex3f(0, 0.25 * 5, 0);
		// 底部顶点：根部位置下方
		glVertex3f(0, -0.25 * 5, 0);
		// 尖端顶点：向右延伸1.5*5（确保在机舱外部）
		glVertex3f(1.5 * 5, 0, 0);
	}
	glEnd();
	glPopMatrix();

	// 飞船背面（线框模式）
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glRotatef(180, 0, 1, 0); // 旋转到背面
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 设置线框模式
	glutSolidCube(1.5 * 5); // 同样尺寸的立方体
	glPopMatrix();

	glPopMatrix();
}
void DrawPlanets() {
	for (size_t i = 0; i < planets.size(); ++i) {
		// 绘制轨道
		glPushMatrix();
		glColor3f(0.5, 0.5, 0.5); // 轨道颜色设置为灰色
		glBegin(GL_LINE_LOOP);
		for (float angle = 0; angle < 2 * 3.1415926; angle += 0.01f) {
			glVertex3f(planets[i].orbitRadius * cos(angle), 0, planets[i].orbitRadius * sin(angle));
		}
		glEnd();
		glPopMatrix();

		// 计算行星的当前位置
		planets[i].position = CVector(
			planets[i].orbitRadius * cos(planets[i].angle),
			0,
			planets[i].orbitRadius * sin(planets[i].angle)
		);

		// 绘制行星
		glPushMatrix();
		glTranslatef(planets[i].position.x, planets[i].position.y, planets[i].position.z);
		glRotatef(planets[i].angle, 0, 1, 0); // 自转

		if (wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// 启用并绑定纹理（仅在非线框模式下）
		if (!wireframeMode && planets[i].textureID != 0) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, planets[i].textureID);
		}

		// 设置颜色（仅在没贴图时可见）
		glColor3fv(planets[i].color);

		// 使用 gluSphere 绘制带纹理球体
		GLUquadric* quad = gluNewQuadric();
		gluQuadricTexture(quad, GL_TRUE);
		gluQuadricNormals(quad, GLU_SMOOTH);
		gluSphere(quad, planets[i].radius, 30, 30);
		gluDeleteQuadric(quad);

		if (!wireframeMode && planets[i].textureID != 0) {
			glDisable(GL_TEXTURE_2D);
		}

		glPopMatrix();
	}

	// 土星环绘制（不加纹理）
	if (!wireframeMode) {
		glPushMatrix();
		glTranslatef(planets[7].position.x, planets[7].position.y, planets[7].position.z);
		glColor3f(1.0, 1.0, 0.0); // 土星环颜色
		glutSolidTorus(0.1f * 7, 0.5f * 7, 20, 20);
		glPopMatrix();
	}

	// 绘制选中行星线框轮廓
	if (selectedPlanet != -1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPushMatrix();
		glTranslatef(planets[selectedPlanet].position.x, planets[selectedPlanet].position.y, planets[selectedPlanet].position.z);
		glColor3f(1.0, 1.0, 1.0);
		glutSolidSphere(planets[selectedPlanet].radius, 20, 20);
		glPopMatrix();

		// 恢复绘制模式
		glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
	}
}

void draw()
{
    glPushMatrix();
    glRotatef(seta, 1, 1, 0);
    DrawStars();
    glPopMatrix();

    // 绘制行星
    glPushMatrix();
    DrawPlanets();
    glPopMatrix();

    // 绘制飞船
    glPushMatrix();
    DrawShip();
    glPopMatrix();

	// 新增绘制太空人
	glPushMatrix();
	DrawAstronaut();
	glPopMatrix();
}
void myDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glLoadIdentity();  // 重置模型视图矩阵
    /*gluLookAt(7.0, 7.0, 7.0,  // 摄像机位置
        0.0, 0.0, 0.0,  // 观察目标点
        0.0, 1.0, 0.0); // 上方向*/
	if (isInterpolating) {
		// 计算插值进度
		float t = (float)interpolationFrame / interpolationTotalFrames;

		// 插值计算视点位置
		mx = initialPosition.x + (targetPosition.x - initialPosition.x) * t;
		my = initialPosition.y + (targetPosition.y - initialPosition.y) * t;
		mz = initialPosition.z + (targetPosition.z - initialPosition.z) * t;

		// 使用四元数插值计算当前旋转
		currentQuat = initialQuat.Slerp(targetQuat, t);

		// 更新插值帧数
		interpolationFrame++;
		if (interpolationFrame >= interpolationTotalFrames) {
			// 插值完成
			isInterpolating = false;
			isAstronautView = !isAstronautView; // 切换视点模式
		}
	}

	bool bChange = false;
	if (keyState['w'])
	{
		if (isAstronautView)
		{
			//运动
			astronaut.position.z += cos(astronaut.angle * 3.1415 / 180.0) * mspeed;
			astronaut.position.x += sin(astronaut.angle * 3.1415 / 180.0) * mspeed;
			astronaut.position.x = clamp(astronaut.position.x, -0.75f, 0.75f);
			astronaut.position.y = clamp(astronaut.position.y, -0.75f, 0.75f);
			astronaut.position.z = clamp(astronaut.position.z, -0.75f, 0.75f);
		}
		else if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, -mspeed, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[4] * mspeed;
			my += g_IEyeMat[5] * mspeed;
			mz += g_IEyeMat[6] * mspeed;
		}
	}
	if (keyState['s'])
	{
		if (isAstronautView)
		{
			//运动
			astronaut.position.z -= cos(astronaut.angle * 3.1415 / 180.0) * mspeed;
			astronaut.position.x -= sin(astronaut.angle * 3.1415 / 180.0) * mspeed;
			astronaut.position.x = clamp(astronaut.position.x, -0.75f, 0.75f);
			astronaut.position.y = clamp(astronaut.position.y, -0.75f, 0.75f);
			astronaut.position.z = clamp(astronaut.position.z, -0.75f, 0.75f);
		}
		else if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, mspeed, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[4] * mspeed;
			my -= g_IEyeMat[5] * mspeed;
			mz -= g_IEyeMat[6] * mspeed;
		}
	}
	if (keyState['a'])
	{
		if (isAstronautView)
		{
			//运动
			astronaut.angle += rspeed;
			astronaut.position.x = clamp(astronaut.position.x, -0.75f, 0.75f);
			astronaut.position.y = clamp(astronaut.position.y, -0.75f, 0.75f);
			astronaut.position.z = clamp(astronaut.position.z, -0.75f, 0.75f);
		}
		else if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(mspeed, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[0] * mspeed;
			my -= g_IEyeMat[1] * mspeed;
			mz -= g_IEyeMat[2] * mspeed;
		}
	}
	if (keyState['d'])
	{
		if (isAstronautView)
		{
			//运动
			astronaut.angle -= rspeed;
			astronaut.position.x = clamp(astronaut.position.x, -0.75f, 0.75f);
			astronaut.position.y = clamp(astronaut.position.y, -0.75f, 0.75f);
			astronaut.position.z = clamp(astronaut.position.z, -0.75f, 0.75f);
		}
		else if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(-mspeed, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[0] * mspeed;
			my += g_IEyeMat[1] * mspeed;
			mz += g_IEyeMat[2] * mspeed;
		}

	}
	if (keyState['q'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, 0, mspeed);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[8] * mspeed;
			my -= g_IEyeMat[9] * mspeed;
			mz -= g_IEyeMat[10] * mspeed;
		}
	}
	if (keyState['e'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, 0, -mspeed);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[8] * mspeed;
			my += g_IEyeMat[9] * mspeed;
			mz += g_IEyeMat[10] * mspeed;
		}
	}
	if (keyState['i'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 1, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rx += rspeed;
			bChange = true;
		}
	}
	if (keyState['k'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 1, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rx -= rspeed;
			bChange = true;
		}
	}
	if (keyState['j'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 0, 1, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			ry += rspeed;
			bChange = true;
		}
	}
	if (keyState['l'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 0, 1, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			ry -= rspeed;
			bChange = true;
		}
	}
	if (keyState['u'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 0, 0, 1);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rz += rspeed;
			bChange = true;
		}
	}
	if (keyState['o'])
	{
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 0, 0, 1);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rz -= rspeed;
			bChange = true;
		}
	}
	if (keyState['0'])
	{
		mode = 1 - mode;
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rz, 0, 0, 1);
			glRotatef(-rx, 1, 0, 0);
			glRotatef(-ry, 0, 1, 0);
			glTranslatef(-mx, -my, -mz);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		printf("mode:%d\n", mode);
	}
	if (bChange)//计算视点矩阵的逆矩阵
	{
		glPushMatrix();
		glLoadIdentity();
		glRotatef(ry, 0, 1, 0);
		glRotatef(rx, 1, 0, 0);
		glRotatef(rz, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, g_IEyeMat);
		glPopMatrix();
	}
	glutPostRedisplay(); // 重新绘制窗口
    glPushMatrix();
    SetView();
    draw();
    glPopMatrix();
    glutSwapBuffers();
}
void DrawStars() {
    glColor3f(1.0f, 1.0f, 1.0f); // 设置星星颜色为白色
    glPointSize(2.0f); // 设置星星大小
    glBegin(GL_POINTS);
    for (int i = 0; i < 100; ++i) {
        float brightness = sin(t + i) * 0.5f + 0.5f; // 计算星星亮度
        glColor3f(brightness, brightness, brightness); // 设置星星亮度
        glVertex3fv(stars[i]); // 绘制星星
    }
    glEnd();
}
void myReshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, GLfloat(w) / h, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void myKeyboard(unsigned char key, int x, int y) {
	keyState[key] = true; // 设置按键状态为按下
	if (key == 'c' || key == 'C') { // 按下 C 键切换视点模式
		if (!isInterpolating) {
			isInterpolating = true;
			interpolationFrame = 0;

			// 存储初始状态
			initialPosition = CVector(mx, my, mz);
			initialQuat = CQuaternion(cos(rx * PI / 360),
				sin(rx * PI / 360) * sin(ry * PI / 360),
				sin(rx * PI / 360) * cos(ry * PI / 360),
				0); // 初始化四元数

			if (isAstronautView) {
				// 存储目标状态为上帝视角
				targetPosition = CVector(godView_mx, godView_my, godView_mz);
				targetQuat = CQuaternion(cos(godView_rx * PI / 360),
					sin(godView_rx * PI / 360) * sin(godView_ry * PI / 360),
					sin(godView_rx * PI / 360) * cos(godView_ry * PI / 360),
					0);
			}
			else {
				// 存储目标状态为宇航员视角
				targetPosition = CVector(spaceship.position.x, spaceship.position.y + 0.5f, spaceship.position.z - 1.0f);
				// 使用四元数表示朝向飞船的旋转
				targetQuat = CQuaternion(cos(-90 * PI / 360),
					sin(-90 * PI / 360) * sin(spaceship.angleY * PI / 360),
					sin(-90 * PI / 360) * cos(spaceship.angleY * PI / 360),
					0);
			}
		}
	}

    glutPostRedisplay(); // 请求重新绘制窗口
    /*if (key == ' ') { // 如果按下空格键

        t = 0; // 重置时间变量
        for (int i = 0; i < 100; ++i) {
            stars[i].Set(rand() % 40 - 20, rand() % 40 - 20, rand() % 40 - 20, 1); // 重新生成随机星星位置
        }
    }
	printf("%c Down\n", key);
	bool bChange = false;
	switch (key)
	{
	case 'w':
		//my+=mspeed;
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, -mspeed, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[4] * mspeed;
			my += g_IEyeMat[5] * mspeed;
			mz += g_IEyeMat[6] * mspeed;
		}
		break;
	case 's':
		//my-=mspeed;	
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, mspeed, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[4] * mspeed;
			my -= g_IEyeMat[5] * mspeed;
			mz -= g_IEyeMat[6] * mspeed;
		}

		break;
	case 'a':
		//mx-=mspeed;
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(mspeed, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[0] * mspeed;
			my -= g_IEyeMat[1] * mspeed;
			mz -= g_IEyeMat[2] * mspeed;
		}

		break;
	case 'd':
		//mx+=mspeed;
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(-mspeed, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[0] * mspeed;
			my += g_IEyeMat[1] * mspeed;
			mz += g_IEyeMat[2] * mspeed;
		}

		break;
	case 'q':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, 0, mspeed);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx -= g_IEyeMat[8] * mspeed;
			my -= g_IEyeMat[9] * mspeed;
			mz -= g_IEyeMat[10] * mspeed;
		}
		//mz-=mspeed;

		break;
	case 'e':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(0, 0, -mspeed);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			mx += g_IEyeMat[8] * mspeed;
			my += g_IEyeMat[9] * mspeed;
			mz += g_IEyeMat[10] * mspeed;
		}
		//mz+=mspeed;

		break;
	case 'i':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 1, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rx += rspeed;
			bChange = true;
		}
		break;
	case 'k':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 1, 0, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rx -= rspeed;
			bChange = true;
		}

		break;
	case 'j':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 0, 1, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			ry += rspeed;
			bChange = true;
		}

		break;
	case 'l':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 0, 1, 0);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			ry -= rspeed;
			bChange = true;
		}

		break;
	case 'u':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(rspeed, 0, 0, 1);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rz += rspeed;
			bChange = true;
		}

		break;
	case 'o':
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rspeed, 0, 0, 1);
			glMultMatrixf(g_EyeMat);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		else
		{
			rz -= rspeed;
			bChange = true;
		}

		break;
	case '0':
		mode = 1 - mode;
		if (mode == 0)
		{
			glPushMatrix();
			glLoadIdentity();
			glRotatef(-rz, 0, 0, 1);
			glRotatef(-rx, 1, 0, 0);
			glRotatef(-ry, 0, 1, 0);
			glTranslatef(-mx, -my, -mz);
			glGetFloatv(GL_MODELVIEW_MATRIX, g_EyeMat);
			glPopMatrix();
		}
		printf("mode:%d\n", mode);
		break;
	case '=':
		mspeed *= 1.1;
		printf("mspeed:%.1f\n", mspeed);
		break;
	case '-':
		mspeed *= 0.9;
		printf("mspeed:%.1f\n", mspeed);
		break;
	}
	if (bChange)//计算视点矩阵的逆矩阵
	{
		glPushMatrix();
		glLoadIdentity();
		glRotatef(ry, 0, 1, 0);
		glRotatef(rx, 1, 0, 0);
		glRotatef(rz, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, g_IEyeMat);
		glPopMatrix();
	}
		glutPostRedisplay(); */
	
}
void mySpecialKeyboard(int key, int x, int y) {
    if (key == GLUT_KEY_F1) { // 按下 F1 键
        // 切换填充模式和线框模式
        wireframeMode = !wireframeMode;
    }
    glutPostRedisplay(); // 重新绘制窗口
}
void KeyUp(unsigned char key, int x, int y) {
	keyState[key] = false; // 设置按键状态为未按下
	glutPostRedisplay(); // 请求重新绘制窗口
}
int main(int argc, char* argv[])
{
    Calculate2();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    srand((unsigned int)time(NULL)); // 初始化随机数种子

    for (int i = 0; i < 100; ++i) {
        stars[i].Set(rand() % 40 - 20, rand() % 40 - 20, rand() % 40 - 20, 1);
        starBrightness[i] = rand() % 100 / 100.0f;
    }// 初始化星星的位置
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("图形学作业4");
    glutDisplayFunc(&myDisplay);
    glutTimerFunc(33, myTimerFunc, 0);
    glutReshapeFunc(&myReshape);
    glutKeyboardFunc(&myKeyboard);
    glutMouseFunc(&mouse);
    glutSpecialFunc(mySpecialKeyboard); // 注册特殊键回调函数
    glutKeyboardUpFunc(&KeyUp);
    SetRC();
    glutMainLoop();
    return 0;
}
