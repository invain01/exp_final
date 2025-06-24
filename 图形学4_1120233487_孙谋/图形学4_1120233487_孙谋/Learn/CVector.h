#pragma once
#include"math.h"
class CEuler;
class CVector {
public:
    float x, y, z, w; // �ĸ�����

    CVector() : x(0), y(0), z(0), w(0) {}
    CVector(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    void Set(float x, float y, float z, float w ) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    ~CVector() {};

    // ���ؼӷ�������
    CVector operator+(const CVector& p) const {
        return CVector(x + p.x, y + p.y, z + p.z, w + p.w);
    }

    // ���ؼ���������
    CVector operator-(const CVector& p) const {
        return CVector(x - p.x, y - p.y, z - p.z, w - p.w);
    }

    // ���ظ�ֵ������
    CVector& operator=(const CVector& p) {
        if (this != &p) {
            x = p.x;
            y = p.y;
            z = p.z;
            w = p.w;
        }
        return *this;
    }

    // ������Ȳ�����
    bool operator==(const CVector& p) const {
        return (x == p.x) && (y == p.y) && (z == p.z) && (w == p.w);
    }

    // ���ز��Ȳ�����
    bool operator!=(const CVector& p) const {
        return !(*this == p);
    }

    // �������˲�����
    CVector operator*(float scalar) const {
        return CVector(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    // ������Ԫ���˲�������������������
    friend CVector operator*(float scalar, const CVector& p) {
        return CVector(p.x * scalar, p.y * scalar, p.z * scalar, p.w * scalar);
    }

    // ��˲���
    float dotMul(const CVector& n) const {
        return x * n.x + y * n.y + z * n.z + w * n.w;
    }

    // ��˲���������������ά������
    CVector crossMul(const CVector& n) const {
        return CVector(y * n.z - z * n.y, z * n.x - x * n.z, x * n.y - y * n.x);
    }

    // ��λ������
    CVector Normalize() const {
        float length = len();
        if (length != 0) {
            return *this * (1.0f / length);
        }
        return *this;
    }

    // ��ģ����
    float len() const {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    // ͶӰ����
    CVector project(const CVector& n) const {
        float dotProduct = dotMul(n);
        float lengthSquared = n.len() * n.len();
        if (lengthSquared != 0) {
            return n * (dotProduct / lengthSquared);
        }
        return CVector(0, 0, 0, 0);
    }
    CEuler ToEuler() const ;

    operator float* () { return &x; } // �Զ�����ת��
};

bool Calculate();