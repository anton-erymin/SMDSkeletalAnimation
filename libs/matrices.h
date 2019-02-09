#include "stdafx.h"

#define DEGREESPERRAD	57.295780490442968321226

typedef struct
{
	float m[16];
} Matrix;

void MatrixIdentity(Matrix *res)
{
	for (int i = 0; i < 16; i++)
		res->m[i] = 0.0f;

	res->m[0] = 1.0f;
	res->m[5] = 1.0f;
	res->m[10] = 1.0f;
	res->m[15] = 1.0f;
}

void MatrixMult(Matrix *res, Matrix *m1, Matrix *m2)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			res->m[4*i+j] = m1->m[4*i + 0]*m2->m[0  + j] + 
							m1->m[4*i + 1]*m2->m[4  + j] + 
							m1->m[4*i + 2]*m2->m[8  + j] + 
							m1->m[4*i + 3]*m2->m[12 + j];
}

void MatrixTranslation(Matrix *res, float xt, float yt, float zt)
{
	MatrixIdentity(res);
	res->m[3] = xt;
	res->m[7] = yt;
	res->m[11] = zt;
}

void MatrixRotation(Matrix *res, float xr, float yr, float zr)
{
	/*xr *= DEGREESPERRAD;
	yr *= DEGREESPERRAD;
	zr *= DEGREESPERRAD;

	glPushMatrix();
	glLoadIdentity();
	glRotatef(xr, 1.0f, 0.0f, 0.0f); 
	glRotatef(yr, 0.0f, 1.0f, 0.0f);
	glRotatef(zr, 0.0f, 0.0f, 1.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, res->m);
	glPopMatrix();*/

	/*float A = cos(xr);
	float B = sin(xr);
	float C = cos(yr);
	float D = sin(yr);
	float E = cos(zr);
	float F = sin(zr);

	float AD = A*D;
	float BD = B*D;

	for (int i = 0; i < 16; i++)
		res->m[i] = 0.0f;

	res->m[0] = C*E;
	res->m[1] = -C*F;
	res->m[2] = -D;
	res->m[4] = -BD*E + A*F;
	res->m[5] = BD*F + A*E;
	res->m[6] = -B*C;
	res->m[8] = AD*E + B*F;
	res->m[9] = -AD*F + B*E;
	res->m[10] = A*C;
	res->m[15] = 1.0f;*/

	/*res->m[0] = C*E;
	res->m[4] = -C*F;
	res->m[8] = -D;
	res->m[1] = -BD*E + A*F;
	res->m[5] = BD*F + A*E;
	res->m[9] = -B*C;
	res->m[2] = AD*E + B*F;
	res->m[10] = -AD*F + B*E;
	res->m[10] = A*C;
	res->m[15] = 1.0f;*/

	float c1 = cos(xr);
	float s1 = sin(xr);
	float c2 = cos(yr);
	float s2 = sin(yr);
	float c3 = cos(zr);
	float s3 = sin(zr);

	Matrix Rx, Ry, Rz;
	MatrixIdentity(&Rx);
	MatrixIdentity(&Ry);
	MatrixIdentity(&Rz);

	Rx.m[5] = c1;
	Rx.m[6] = -s1;
	Rx.m[9] = s1;
	Rx.m[10] = c1;

	Ry.m[0] = c2;
	Ry.m[2] = -s2;
	Ry.m[8] = s2;
	Ry.m[10] = c2;

	Rz.m[0] = c3;
	Rz.m[1] = -s3;
	Rz.m[4] = s3;
	Rz.m[5] = c3;

	Matrix t;
	MatrixMult(&t, &Rz, &Ry);
	MatrixMult(res, &t, &Rx);
}

void MatrixApply(Matrix m, float *x, float *y, float *z)
{
	float xx = *x;
	float yy = *y;
	float zz = *z;

	*x = m.m[0]*xx + m.m[1]*yy + m.m[2]*zz + m.m[3];
	*y = m.m[4]*xx + m.m[5]*yy + m.m[6]*zz + m.m[7];
	*z = m.m[8]*xx + m.m[9]*yy + m.m[10]*zz + m.m[11];
}
void MatrixInvApply(Matrix m, float *x, float *y, float *z)
{
	float xx = *x;
	float yy = *y;
	float zz = *z;

	xx = xx - m.m[3];
	yy = yy - m.m[7];
	zz = zz - m.m[11];

	*x = m.m[0]*xx + m.m[4]*yy + m.m[8]*zz;
	*y = m.m[1]*xx + m.m[5]*yy + m.m[9]*zz;
	*z = m.m[2]*xx + m.m[6]*yy + m.m[10]*zz;

	/**x = *x - m.m[3];
	*y = *y - m.m[7];
	*z = *z - m.m[11];*/
}

void MatrixInterpolate(float interp, Matrix m1, Matrix m2, Matrix *res)
{
	res->m[0] = m1.m[0] + interp * (m2.m[0] - m1.m[0]);
	res->m[1] = m1.m[1] + interp * (m2.m[1] - m1.m[1]);
	res->m[2] = m1.m[2] + interp * (m2.m[2] - m1.m[2]);
	res->m[3] = m1.m[3] + interp * (m2.m[3] - m1.m[3]);
	res->m[4] = m1.m[4] + interp * (m2.m[4] - m1.m[4]);
	res->m[5] = m1.m[5] + interp * (m2.m[5] - m1.m[5]);
	res->m[6] = m1.m[6] + interp * (m2.m[6] - m1.m[6]);
	res->m[7] = m1.m[7] + interp * (m2.m[7] - m1.m[7]);
	res->m[8] = m1.m[8] + interp * (m2.m[8] - m1.m[8]);
	res->m[9] = m1.m[9] + interp * (m2.m[9] - m1.m[9]);
	res->m[10] = m1.m[10] + interp * (m2.m[10] - m1.m[10]);
	res->m[11] = m1.m[11] + interp * (m2.m[11] - m1.m[11]);
};