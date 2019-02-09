#include <math.h>
#include "libs\opengl.h"
#include "libs\camera.h"
#include "libs\fpscounter.h"
#include "libs\md2model.h"
#include "libs\ms3dmodel.h"
#include "libs\smdmodel.h"
#include "libs\list.h"
#include "libs\anim_geometry.h"

bool keys[256];

Camera *sceneCamera;
FPSCounter *fps;
char fpsstr[32];

SMDModel *m;

int lastTime;
int animfps;

void ProcessKeyboard()
{
	GLfloat speed = 10.0f;//fps->GetFPS();
 	GLfloat coeff = 0.1f;
	if (keys[VK_UP]) sceneCamera->Move(coeff*speed);
	if (keys[VK_DOWN]) sceneCamera->Move(-coeff*speed);
	if (keys[VK_LEFT]) sceneCamera->Strafe(coeff*speed);
	if (keys[VK_RIGHT]) sceneCamera->Strafe(-coeff*speed);
	if (keys[VK_ESCAPE]) PostQuitMessage(0);
	if (keys[VK_INSERT])
	{
		Vector v = sceneCamera->GetPosition();
		v.y += 1.0f;
		sceneCamera->SetPosition(v.x, v.y, v.z);
	}
	if (keys[VK_DELETE])
	{
		Vector v = sceneCamera->GetPosition();
		v.y -= 1.0f;
		sceneCamera->SetPosition(v.x, v.y, v.z);
	}
}

void ProcessMouse()
{
	POINT p; 
	GetCursorPos(&p);
	SetCursorPos(512, 384);

	int delta_x = p.x - 512;
	int delta_y = p.y - 384;
	

}
void InitScene()
{
	sceneCamera = new Camera();
	sceneCamera->SetPosition(0.0f, 30.0f, 100.0f);

	fps = new FPSCounter();
	fps->Init();

	m = new SMDModel();
	m->LoadFromFile("terror.smd");

	SMDAnimation *anim = new SMDAnimation(m->numNodes);
	anim->LoadFromFile("head.smd");

	m->currentAnim = anim;
	m->curframe = 0;

	animfps = 25;
}


void DrawCoordinateAxes()
{
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
		glColor3ub(255, 0, 0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(100.0f, 0.0f, 0.0f);

		glColor3ub(0, 255, 0);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 100.0f, 0.0f);

		glColor3ub(0, 0, 255);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();
	glEnable(GL_LIGHTING);
}  

void DrawScene(HWND hwnd)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessKeyboard();

	sceneCamera->MouseLook();
	sceneCamera->SetCamera();

	DrawCoordinateAxes();

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	int time = GetTickCount();
	float interp = (time - lastTime) / (1000.0f/animfps);
	if (interp > 1.0f) interp = 1.0f;
	m->Draw(interp);

	
	if (time - lastTime > 1000.0f/animfps)
	{
		lastTime = GetTickCount();

		m->curframe++;
		if (m->curframe >= m->currentAnim->numFrames)
			m->curframe = 0;
	}

	
	

	fps->GetStrFPS(fpsstr);
	SetWindowTextA(hwnd, fpsstr);
	SwapBuffers(hDC);

	
}

