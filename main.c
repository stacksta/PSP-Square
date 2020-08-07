#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspgu.h>
#include <pspgum.h>

#include "common/callbacks.h"
#include "common/vram.h"

PSP_MODULE_INFO("square", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

static unsigned int __attribute__((aligned(16))) list[262144];

struct Vertex
{
	unsigned int color;
	float x, y, z;
};

struct Vertex __attribute__((aligned(16))) vertices[6] =
{
	{0xFFF59E42,-1,-1,-1}, // 3
	{0xFFF59E42, 1,-1,-1}, // 2
	{0xFFF59E42, 1, 1,-1}, // 6

	{0xFFF59E42,-1,-1,-1}, // 3
	{0xFFF59E42, 1, 1,-1}, // 6
	{0xFFF59E42,-1, 1,-1}, // 7
};


int main(int argc, char* argv[])
{
	setupCallbacks();

	//setup GU
	void* fbp0 = getStaticVramBuffer(BUF_WIDTH, SCR_HEIGHT, GU_PSM_8888);
	void* fbp1 = getStaticVramBuffer(BUF_WIDTH, SCR_HEIGHT, GU_PSM_8888);
	void* zbp = getStaticVramBuffer(BUF_WIDTH, SCR_HEIGHT, GU_PSM_4444);

	sceGuInit();

	sceGuStart(GU_DIRECT, list);
		sceGuDrawBuffer(GU_PSM_8888, fbp0, BUF_WIDTH);
		sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, fbp1, BUF_WIDTH);
		sceGuDepthBuffer(zbp, BUF_WIDTH);
		sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));//virtual coordinate offset (0 to 4095)
		sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
		sceGuDepthRange(/*near*/65535, /*far*/0);

		sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuDepthFunc(GU_GEQUAL);
		
		//sceGuEnable(GU_DEPTH_TEST);
		sceGuFrontFace(GU_CW);//clockwise primitives are not culled

		sceGuShadeModel(GU_SMOOTH);

		// sceGuEnable(GU_CULL_FACE);
		// sceGuEnable(GU_TEXTURE_2D);
		sceGuEnable(GU_CLIP_PLANES);

	sceGuFinish();

	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	int val = 0;

	ScePspFVector3 pos = {240.0f, 136.0f, 0.0f};
	ScePspFVector3 scale = { 30.0f, 30.0f, 1.0f};

	while(running())
	{
		sceGuStart(GU_DIRECT, list);

			//clear screen
			sceGuClearColor(0xFF554433);
			sceGuClearDepth(0);
			sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

			//setup matrices for square
			sceGumMatrixMode(GU_PROJECTION);
			sceGumLoadIdentity();
			sceGumOrtho(0, 480, 272, 0, -1, 1);//left, right, bottom, top, near, far

			sceGumMatrixMode(GU_VIEW);
			sceGumLoadIdentity();

			sceGumMatrixMode(GU_MODEL);
			sceGumLoadIdentity();

			//draw 
			sceGumTranslate(&pos);
			sceGumScale(&scale);

			sceGumRotateZ(val*0.03);


			sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 6, 0, vertices);



		sceGuFinish();
		sceGuSync(0, 0);

		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();

		val++;

	}
	sceGuTerm();

	sceKernelExitGame();

	return 0;
}