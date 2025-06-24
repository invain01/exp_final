
#include "stdafx.h"
#include "glut.h"
#include "math.h"
#include "CVector.h"
#include "CMatrix.h"
#include <ctime>
#include <vector>
#include <algorithm> 
#include "CEuler.h"
bool keyState[256] = { false }; // ��ʼ�����м���״̬Ϊδ����
bool isAstronautView = false; // ��ǰ�Ƿ�Ϊ̫�����ӽ�
// �ӵ�λ�úͷ���
float mx = 0, my = 5, mz = 10, rx = -0.25, ry = 0, rz = 0; // ƽ�ƺ���ת
float godView_mx = 0, godView_my = 5, godView_mz = 10;
float godView_rx = -0.25, godView_ry = 0, godView_rz = 0;
float sx = 1, sy = 1, sz = 1; // ����
float mspeed = 0.003, rspeed = 0.02;
float g_IEyeMat[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }, g_EyeMat[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
int mode = 1; // 0: ����ģʽ��1: ֱ��ģʽ
bool isInterpolating = false; // �Ƿ����ڽ��в�ֵ
int interpolationFrame = 0; // ��ǰ��ֵ֡��
const int interpolationTotalFrames = 300; // �ܲ�ֵ֡��
CVector initialPosition, targetPosition; // ��ʼ��Ŀ���ӵ�λ��
float initialRx, initialRy, initialRz; // ��ʼ��ת�Ƕ�
float targetRx, targetRy, targetRz; // Ŀ����ת�Ƕ�
CQuaternion initialQuat, targetQuat; // ��ʼ��Ŀ����Ԫ��
CQuaternion currentQuat;            // ��ǰ��Ԫ��

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
	CVector position; // ̫����λ��
	float angle; // ̫���˵���ת�Ƕ�
	float step; // ̫���˵����߲���
};
Astronaut astronaut = { CVector(0, 0, 0), 0, 0 };
CVector v; // ����ٶ�����
CVector stars[100]; // ���ǵ�λ��
CVector points[30][60];
CVector pos;//���λ��
float starBrightness[100]; // ���ǵ�����
float t = 0; // ʱ����������ڿ����������ȱ仯
void myDisplay(void);
void DrawStars();
void myKeyboard();
void DrawPlanets();
void DrawShip();
void DrawText(const char* text, float x, float y, float z, void* font);
void rotateConeToDirection(float vx, float vy, float vz);
float seta = 0;//��ת�ٶ�
float sp = 0.02;//�ٶȴ�С
struct Planet {
    CVector position; // ����λ��
    float radius; // ���ǰ뾶
    float orbitRadius; // ��ת����뾶
    float orbitSpeed; // ��ת�ٶ�
    float rotationSpeed; // ��ת�ٶ�
    float angle; // ��ǰ�Ƕ�
    CVector color; // ������ɫ
};

struct Spaceship {
    CVector position; // �ɴ�λ��
    float angleX, angleY; // �ɴ�����ת�Ƕ�
    float speed; // �ɴ��ٶ�
    CVector direction; // �ɴ����ƶ�����
    int targetPlanet; // Ŀ�����ǵ�����
};

Spaceship spaceship = { CVector(0, 0, 0), 0, 0, 0.02, CVector(0, 0, 0), -1 };

// ȫ�ֱ���
std::vector<Planet> planets = {
	{CVector(0, 0, 0), 0.5 * 5, 0, 0, 0, 0, CVector(1.0, 1.0, 0.0)}, // ̫������ɫ��
	{CVector(0, 0, 0), 0.2 * 5, 1.5 * 5, 0.01, 0.02, 0, CVector(0.8, 0.8, 0.8)}, // ˮ�ǣ���ɫ��
	{CVector(0, 0, 0), 0.3 * 5, 2.0 * 5, 0.008, 0.015, 0, CVector(0.7, 0.6, 0.2)}, // ���ǣ��ƺ�ɫ��
	{CVector(0, 0, 0), 0.4 * 5, 2.5 * 5, 0.006, 0.01, 0, CVector(0.2, 0.2, 0.9)}, // ������ɫ��
	{CVector(0, 0, 0), 0.1 * 5, 2.6 * 5, 0.007, 0.012, 0, CVector(0.8, 0.8, 0.8)}, // ���򣨻�ɫ��
	{CVector(0, 0, 0), 0.35 * 5, 3.0 * 5, 0.005, 0.008, 0, CVector(0.9, 0.2, 0.2)}, // ���ǣ���ɫ��
	{CVector(0, 0, 0), 0.6 * 5, 4.0 * 5, 0.004, 0.006, 0, CVector(0.9, 0.9, 0.2)}, // ľ�ǣ�ǳ��ɫ��
	{CVector(0, 0, 0), 0.55 * 5, 5.0 * 5, 0.003, 0.005, 0, CVector(0.8, 0.6, 0.2)}, // ���ǣ��ƺ�ɫ��
};
int selectedPlanet = -1; // ��ǰѡ�е�����
bool wireframeMode = false; // �Ƿ�Ϊ�߿�ģʽ
void myTimerFunc(int val)
{
    t += 0.033f;
    seta += 0.1f;
    for (size_t i = 1; i < planets.size(); ++i) {
        planets[i].angle += planets[i].orbitSpeed;
    }
    if (spaceship.targetPlanet != -1) {
        // ����ɴ���Ŀ�����ǵľ���
        float distance = (planets[spaceship.targetPlanet].position - spaceship.position).len();
        if (distance > spaceship.speed) {
            // �ط��������ƶ��ɴ�
            spaceship.position = spaceship.position + spaceship.direction * spaceship.speed;
        }
        else {
            // �������С���ٶȣ�ֱ�ӵ���Ŀ��λ��
            spaceship.position = planets[spaceship.targetPlanet].position;
            spaceship.targetPlanet = -1; // ����Ŀ������
        }
    }
    else {
        // ���û��Ŀ�����ǣ�����ǰ�����ƶ�
        CVector direction = CVector(
            sin(spaceship.angleY * 3.1415 / 180.0),
            0,
            cos(spaceship.angleY * 3.1415 / 180.0)
        );
        direction.Normalize();
        spaceship.position = spaceship.position + direction * spaceship.speed;
    }
	// �����Ա��ͼ�µ��ӵ�λ��
	// �����Ա��ͼ�µ��ӵ�λ��
	if (isAstronautView) {
		mx = spaceship.position.x; // ����ͷλ�ڷɴ��ڲ�
		my = spaceship.position.y + 0.5f; // �ӵ���΢���ڷɴ��ײ�
		mz = spaceship.position.z - 1.0f; // �ӵ�λ�ڷɴ���һ������
		ry = spaceship.angleY; // �ӵ㷽����ɴ�����һ��
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
    if (tca < 0) return false; // ���߷��������ķ����෴���޽���

    float d2 = L.dotMul(L) - tca * tca;
    float r2 = sphereRadius * sphereRadius;
    //printf("d2=%f,r2=%f\n",d2, r2);
    if (d2 > r2) return false; // ���������岻�ཻ

    float thc = sqrt(r2 - d2);
    t = tca - thc; // ����Ľ���
    return true;
}
void SetView() {
	if (isAstronautView) {
		// ̫�����ӽ�
		glTranslatef(-mx, -my, -mz);
		glRotatef(-rz, 0, 0, 1);
		glRotatef(-rx, 1, 0, 0);
		glRotatef(-ry, 0, 1, 0);
	}
	else {
		// �ϵ��ӽ�
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
	float step = 2 * PI / slices; // ÿ����Ƭ�ĽǶ�
	float halfHeight = height / 2.0f; // Բ�����һ��߶�

	// ����Բ����Ĳ���
	for (int i = 0; i < slices; ++i) {
		glBegin(GL_QUADS);
		{
			// ��Բ
			glVertex3f(radius * cos(i * step), halfHeight, radius * sin(i * step));
			glVertex3f(radius * cos((i + 1) * step), halfHeight, radius * sin((i + 1) * step));
			// ��Բ
			glVertex3f(radius * cos((i + 1) * step), -halfHeight, radius * sin((i + 1) * step));
			glVertex3f(radius * cos(i * step), -halfHeight, radius * sin(i * step));
		}
		glEnd();
	}

	// ����Բ����Ķ���
	glBegin(GL_POLYGON);
	for (int i = 0; i < slices; ++i) {
		glVertex3f(radius * cos(i * step), halfHeight, radius * sin(i * step));
	}
	glEnd();

	// ����Բ����ĵײ�
	glBegin(GL_POLYGON);
	for (int i = 0; i < slices; ++i) {
		glVertex3f(radius * cos(i * step), -halfHeight, radius * sin(i * step));
	}
	glEnd();
}
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // ��ȡ�����λ�õ���Ļ����
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        int winWidth = viewport[2];
        int winHeight = viewport[3];
        // ���������ת��Ϊ NDC ����
        float ndcX = (2.0f * x) / winWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * y) / winHeight;
        // ����Ȼ�������ȡ Z ֵ
        float z, h;
        glReadPixels(x, winHeight - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
        glReadPixels(7.0f, winHeight - 7.0f, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &h);
        // �� NDC ����ת��Ϊ��������
        CVector rayOrigin(7 * 2 / winWidth - 1.0f, 1 - (2.0f * 7) / winHeight, h); // �����λ��
        CVector rayDirection(ndcX, ndcY, z);
        rayDirection.Normalize();

        // ���������ÿ�����ǵĽ���
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

        // ����ѡ�е�����
        selectedPlanet = closestPlanet;
    }
    glutPostRedisplay();
}
void DrawAstronaut() {
	glPushMatrix();
	glTranslatef(spaceship.position.x, spaceship.position.y, spaceship.position.z); // ��ɴ��ƶ�
	glTranslatef(astronaut.position.x, astronaut.position.y, astronaut.position.z); // ���λ��

	// ͷ������ɫ��
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // ��ɫ
	glTranslatef(0, 0.5, 0);
	glutSolidSphere(0.2, 20, 20);
	glPopMatrix();

	// ���壨��ɫ��
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0); // ��ɫ
	glTranslatef(0, 0, 0);
	glutSolidCube(0.5);
	glPopMatrix();

	// ���ȣ���ɫ��
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0); // ��ɫ
	glTranslatef(-0.1, -0.25, 0);
	glRotatef(astronaut.step, 1, 0, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// ���ȣ���ɫ��
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0); // ��ɫ
	glTranslatef(0.1, -0.25, 0);
	glRotatef(-astronaut.step, 1, 0, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// ��ۣ���ɫ��
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // ��ɫ
	glTranslatef(-0.25, 0.25, 0);
	glutSolidCylinder(0.1, 0.5, 20, 20);
	glPopMatrix();

	// �ұۣ���ɫ��
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // ��ɫ
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

	// �ɴ�ͷ��
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0); // ��ɫ
	glTranslatef(0, 0.5 * 5, 0);
	glutSolidSphere(0.3 * 5, 20, 20);
	glPopMatrix();

	// �ɴ�����
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0); // ��ɫ
	glTranslatef(0, 0, 0);
	glutSolidCube(1.5 * 5); // ���ճߴ�
	glPopMatrix();

	// �������������죩
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // ��ɫ
	// ����λ�ڻ�������Ե�⣨���տ��1.5*5��������0������Ե-0.75*5����������0.1��
	glTranslatef(-0.75 * 5 - 0.1, 0, 0);

	glBegin(GL_TRIANGLES);
	{
		// �������㣺����λ���Ϸ�
		glVertex3f(0, 0.25 * 5, 0);
		// �ײ����㣺����λ���·�
		glVertex3f(0, -0.25 * 5, 0);
		// ��˶��㣺��������1.5*5��ȷ���ڻ����ⲿ��
		glVertex3f(-1.5 * 5, 0, 0);
	}
	glEnd();
	glPopMatrix();

	// �Ҳ�����������죩
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0); // ��ɫ
	// ����λ�ڻ����Ҳ��Ե�⣨�Ҳ��Ե0.75*5����������0.1��
	glTranslatef(0.75 * 5 + 0.1, 0, 0);

	glBegin(GL_TRIANGLES);
	{
		// �������㣺����λ���Ϸ�
		glVertex3f(0, 0.25 * 5, 0);
		// �ײ����㣺����λ���·�
		glVertex3f(0, -0.25 * 5, 0);
		// ��˶��㣺��������1.5*5��ȷ���ڻ����ⲿ��
		glVertex3f(1.5 * 5, 0, 0);
	}
	glEnd();
	glPopMatrix();

	// �ɴ����棨�߿�ģʽ��
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glRotatef(180, 0, 1, 0); // ��ת������
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // �����߿�ģʽ
	glutSolidCube(1.5 * 5); // ͬ���ߴ��������
	glPopMatrix();

	glPopMatrix();
}
void DrawPlanets() {
    for (size_t i = 0; i < planets.size(); ++i) {
        // ���ƹ��
        glPushMatrix();
        glColor3f(0.5, 0.5, 0.5); // �����ɫ����Ϊ��ɫ
        glBegin(GL_LINE_LOOP);
        for (float angle = 0; angle < 2 * 3.1415926; angle += 0.01) {
            glVertex3f(planets[i].orbitRadius * cos(angle), 0, planets[i].orbitRadius * sin(angle));
        }
        glEnd();
        glPopMatrix();

        // �������ǵĵ�ǰλ��
        planets[i].position = CVector(
            planets[i].orbitRadius * cos(planets[i].angle),
            0,
            planets[i].orbitRadius * sin(planets[i].angle)
        );

        // ��������
        glPushMatrix();
        glTranslatef(planets[i].position.x, planets[i].position.y, planets[i].position.z);
        glRotatef(planets[i].angle, 0, 1, 0); // ��ת
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glColor3fv(planets[i].color); // ʹ�����ǵ���ɫ
        glutSolidSphere(planets[i].radius, 20, 20);
        glPopMatrix();
    }
	if (!wireframeMode) {
		glPushMatrix();
		glTranslatef(planets[7].position.x, planets[7].position.y, planets[7].position.z);
		glColor3f(1.0, 1.0, 0.0); // ���ǻ�����ɫ
		glutSolidTorus(0.1 * 7, 0.5 * 7, 20, 20); // �ǻ��ڰ뾶����뾶������5��
		glPopMatrix();
	}
    // ����ѡ�е����ǵ��߿���
    if (selectedPlanet != -1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPushMatrix();
        glTranslatef(planets[selectedPlanet].position.x, planets[selectedPlanet].position.y, planets[selectedPlanet].position.z);
        glColor3f(1.0, 1.0, 1.0);
        glutSolidSphere(planets[selectedPlanet].radius, 20, 20);
        glPopMatrix();
        glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
    }
}
void draw()
{
    glPushMatrix();
    glRotatef(seta, 1, 1, 0);
    DrawStars();
    glPopMatrix();

    // ��������
    glPushMatrix();
    DrawPlanets();
    glPopMatrix();

    // ���Ʒɴ�
    glPushMatrix();
    DrawShip();
    glPopMatrix();

	// ��������̫����
	glPushMatrix();
	DrawAstronaut();
	glPopMatrix();
}
void myDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glLoadIdentity();  // ����ģ����ͼ����
    /*gluLookAt(7.0, 7.0, 7.0,  // �����λ��
        0.0, 0.0, 0.0,  // �۲�Ŀ���
        0.0, 1.0, 0.0); // �Ϸ���*/
	if (isInterpolating) {
		// �����ֵ����
		float t = (float)interpolationFrame / interpolationTotalFrames;

		// ��ֵ�����ӵ�λ��
		mx = initialPosition.x + (targetPosition.x - initialPosition.x) * t;
		my = initialPosition.y + (targetPosition.y - initialPosition.y) * t;
		mz = initialPosition.z + (targetPosition.z - initialPosition.z) * t;

		// ʹ����Ԫ����ֵ���㵱ǰ��ת
		currentQuat = initialQuat.Slerp(targetQuat, t);

		// ���²�ֵ֡��
		interpolationFrame++;
		if (interpolationFrame >= interpolationTotalFrames) {
			// ��ֵ���
			isInterpolating = false;
			isAstronautView = !isAstronautView; // �л��ӵ�ģʽ
		}
	}

	bool bChange = false;
	if (keyState['w'])
	{
		if (isAstronautView)
		{
			//�˶�
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
			//�˶�
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
			//�˶�
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
			//�˶�
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
	if (bChange)//�����ӵ����������
	{
		glPushMatrix();
		glLoadIdentity();
		glRotatef(ry, 0, 1, 0);
		glRotatef(rx, 1, 0, 0);
		glRotatef(rz, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW_MATRIX, g_IEyeMat);
		glPopMatrix();
	}
	glutPostRedisplay(); // ���»��ƴ���
    glPushMatrix();
    SetView();
    draw();
    glPopMatrix();
    glutSwapBuffers();
}
void DrawStars() {
    glColor3f(1.0f, 1.0f, 1.0f); // ����������ɫΪ��ɫ
    glPointSize(2.0f); // �������Ǵ�С
    glBegin(GL_POINTS);
    for (int i = 0; i < 100; ++i) {
        float brightness = sin(t + i) * 0.5f + 0.5f; // ������������
        glColor3f(brightness, brightness, brightness); // ������������
        glVertex3fv(stars[i]); // ��������
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
	keyState[key] = true; // ���ð���״̬Ϊ����
	if (key == 'c' || key == 'C') { // ���� C ���л��ӵ�ģʽ
		if (!isInterpolating) {
			isInterpolating = true;
			interpolationFrame = 0;

			// �洢��ʼ״̬
			initialPosition = CVector(mx, my, mz);
			initialQuat = CQuaternion(cos(rx * PI / 360),
				sin(rx * PI / 360) * sin(ry * PI / 360),
				sin(rx * PI / 360) * cos(ry * PI / 360),
				0); // ��ʼ����Ԫ��

			if (isAstronautView) {
				// �洢Ŀ��״̬Ϊ�ϵ��ӽ�
				targetPosition = CVector(godView_mx, godView_my, godView_mz);
				targetQuat = CQuaternion(cos(godView_rx * PI / 360),
					sin(godView_rx * PI / 360) * sin(godView_ry * PI / 360),
					sin(godView_rx * PI / 360) * cos(godView_ry * PI / 360),
					0);
			}
			else {
				// �洢Ŀ��״̬Ϊ�Ա�ӽ�
				targetPosition = CVector(spaceship.position.x, spaceship.position.y + 0.5f, spaceship.position.z - 1.0f);
				// ʹ����Ԫ����ʾ����ɴ�����ת
				targetQuat = CQuaternion(cos(-90 * PI / 360),
					sin(-90 * PI / 360) * sin(spaceship.angleY * PI / 360),
					sin(-90 * PI / 360) * cos(spaceship.angleY * PI / 360),
					0);
			}
		}
	}

    glutPostRedisplay(); // �������»��ƴ���
    /*if (key == ' ') { // ������¿ո��

        t = 0; // ����ʱ�����
        for (int i = 0; i < 100; ++i) {
            stars[i].Set(rand() % 40 - 20, rand() % 40 - 20, rand() % 40 - 20, 1); // ���������������λ��
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
	if (bChange)//�����ӵ����������
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
    if (key == GLUT_KEY_F1) { // ���� F1 ��
        // �л����ģʽ���߿�ģʽ
        wireframeMode = !wireframeMode;
    }
    glutPostRedisplay(); // ���»��ƴ���
}
void KeyUp(unsigned char key, int x, int y) {
	keyState[key] = false; // ���ð���״̬Ϊδ����
	glutPostRedisplay(); // �������»��ƴ���
}
int main(int argc, char* argv[])
{
    Calculate2();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    srand((unsigned int)time(NULL)); // ��ʼ�����������

    for (int i = 0; i < 100; ++i) {
        stars[i].Set(rand() % 40 - 20, rand() % 40 - 20, rand() % 40 - 20, 1);
        starBrightness[i] = rand() % 100 / 100.0f;
    }// ��ʼ�����ǵ�λ��
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("ͼ��ѧ��ҵ4");
    glutDisplayFunc(&myDisplay);
    glutTimerFunc(33, myTimerFunc, 0);
    glutReshapeFunc(&myReshape);
    glutKeyboardFunc(&myKeyboard);
    glutMouseFunc(&mouse);
    glutSpecialFunc(mySpecialKeyboard); // ע��������ص�����
    glutKeyboardUpFunc(&KeyUp);
    SetRC();
    glutMainLoop();
    return 0;
}