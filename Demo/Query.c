/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "chipmunk.h"
#include "ChipmunkDemo.h"

static cpSpace *space;

static cpVect QUERY_START = {0,0};

static void
update(int ticks)
{
	if(ChipmunkDemoRightClick){
		QUERY_START = ChipmunkDemoMouse;
	}
	
	int steps = 1;
	cpFloat dt = 1.0f/60.0f/(cpFloat)steps;
	
	for(int i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

char messageScratchSpace[1024] = {};

static void
draw(void)
{
	ChipmunkDemoDefaultDrawImpl();
	
	char *messageCursor = messageScratchSpace;
	messageCursor[0] = '\0';
	
	cpVect start = QUERY_START;
	cpVect end = ChipmunkDemoMouse;
	ChipmunkDebugDrawSegment(start, end, RGBAColor(0,1,0,1));
	
	messageCursor += sprintf(messageCursor, "Query: Dist(%f) Point%s, ", cpvdist(start, end), cpvstr(end));
	
	cpSegmentQueryInfo segInfo = {};
	if(cpSpaceSegmentQueryFirst(space, start, end, CP_ALL_LAYERS, CP_NO_GROUP, &segInfo)){
		cpVect point = cpSegmentQueryHitPoint(start, end, segInfo);
		
		// Draw red over the occluded part of the query
		ChipmunkDebugDrawSegment(point, end, RGBAColor(1,0,0,1));
		
		// Draw a little blue surface normal
		ChipmunkDebugDrawSegment(point, cpvadd(point, cpvmult(segInfo.n, 16)), RGBAColor(0,0,1,1));
		
		// Draw a little red dot on the hit point.
		ChipmunkDebugDrawPoints(3, 1, &point, RGBAColor(1,0,0,1));

		
		messageCursor += sprintf(messageCursor, "Segment Query: Dist(%f) Normal%s", cpSegmentQueryHitDist(start, end, segInfo), cpvstr(segInfo.n));
	} else {
		messageCursor += sprintf(messageCursor, "Segment Query (None)");
	}
	
	// Draw a red bounding box around the shape under the mouse.
	cpNearestPointQueryInfo nearestInfo = {};
	cpSpaceNearestPointQueryNearest(space, ChipmunkDemoMouse, 100.0, CP_ALL_LAYERS, CP_NO_GROUP, &nearestInfo);
	if(nearestInfo.shape){
		ChipmunkDebugDrawSegment(ChipmunkDemoMouse, nearestInfo.p, RGBAColor(0.5, 0.5, 0.5, 1.0));
		if(nearestInfo.d < 0) ChipmunkDebugDrawBB(cpShapeGetBB(nearestInfo.shape), RGBAColor(1,0,0,1));
	}
}

static cpSpace *
init(void)
{
	QUERY_START = cpvzero;
	
	messageScratchSpace[0] = '\0';
	ChipmunkDemoMessageString = messageScratchSpace;
	
	space = cpSpaceNew();
	cpSpaceSetIterations(space, 5);
	
	{ // add a fat segment
		cpFloat mass = 1.0f;
		cpFloat length = 100.0f;
		cpVect a = cpv(-length/2.0f, 0.0f), b = cpv(length/2.0f, 0.0f);
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForSegment(mass, a, b)));
		cpBodySetPos(body, cpv(0.0f, 100.0f));
		
		cpSpaceAddShape(space, cpSegmentShapeNew(body, a, b, 20.0f));
	}
	
	{ // add a static segment
		cpSpaceAddShape(space, cpSegmentShapeNew(cpSpaceGetStaticBody(space), cpv(0, 300), cpv(300, 0), 0.0f));
	}
	
	{ // add a pentagon
		cpFloat mass = 1.0f;
		const int NUM_VERTS = 5;
		
		cpVect verts[NUM_VERTS];
		for(int i=0; i<NUM_VERTS; i++){
			cpFloat angle = -2*M_PI*i/((cpFloat) NUM_VERTS);
			verts[i] = cpv(30*cos(angle), 30*sin(angle));
		}
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, NUM_VERTS, verts, cpvzero)));
		cpBodySetPos(body, cpv(50.0f, 50.0f));
		
		cpSpaceAddShape(space, cpPolyShapeNew(body, NUM_VERTS, verts, cpvzero));
	}
	
	{ // add a circle
		cpFloat mass = 1.0f;
		cpFloat r = 20.0f;
		
		cpBody *body = cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForCircle(mass, 0.0f, r, cpvzero)));
		cpBodySetPos(body, cpv(100.0f, 100.0f));
		
		cpSpaceAddShape(space, cpCircleShapeNew(body, r, cpvzero));
	}
	
	return space;
}

static void
destroy(void)
{
	ChipmunkDemoFreeSpaceChildren(space);
	cpSpaceFree(space);
}

ChipmunkDemo Query = {
	"Segment Query",
	init,
	update,
	draw,
	destroy,
};
