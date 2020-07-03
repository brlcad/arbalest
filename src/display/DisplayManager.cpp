/*              D I S P L A Y M A N A G E R . C P P
 * BRL-CAD
 *
 * Copyright (c) 2020 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file DisplayManager.cpp */

#include "DisplayManager.h"

#define DM_SOLID_LINE 0
#define DM_DASHED_LINE 1

DisplayManager::DisplayManager(Display *display) : display(display)
{
    setFGColor(0,0,0, 1);
    glLineStipple(1, 0xCF33);
}

bool DisplayManager::DrawVListElementCallback::operator()(BRLCAD::VectorList::Element *element) {
    const  float black[4] = {0.0, 0.0, 0.0, 0.0};
    if (!element) return true;

    switch (element->Type()) {

        case BRLCAD::VectorList::Element::LineDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::LineMove: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineMove *> (element);
            if (vars->first == 0) glEnd();
            vars->first = 0;

            if (displayManager->dmLight && vars->mFlag) {
                vars->mFlag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, displayManager->wireColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

                if (displayManager->dmTransparency) glDisable(GL_BLEND);
            }

            glBegin(GL_LINE_STRIP);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::ModelSpace: {
            if (vars->first == 0) {
                glEnd();
                vars->first = 1;
            }

            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            break;
        }
        case BRLCAD::VectorList::Element::DisplaySpace: {
            auto *e = dynamic_cast<BRLCAD::VectorList::DisplaySpace *> (element);
            glMatrixMode(GL_MODELVIEW);
            glGetDoublev(GL_MODELVIEW_MATRIX, vars->m);

            MAT_TRANSPOSE(vars->mt, vars->m);
            MAT4X3PNT(vars->tlate, vars->mt, e->ReferencePoint().coordinates);

            class Display;
            glPushMatrix();
            glLoadIdentity();
            glTranslated(vars->tlate[0], vars->tlate[1], vars->tlate[2]);
            /* 96 dpi = 3.78 pixel/mm hardcoded */
            glScaled(2. * 3.78 / displayManager->display->getW(),
                     2. * 3.78 / displayManager->display->getH(),
                     1.);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonStart: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonStart *> (element);
            if (displayManager->dmLight && vars->mFlag) {
                vars->mFlag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, displayManager->ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, displayManager->specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, displayManager->diffuseColor);

                switch (displayManager->dmLight) {
                    case 1:
                        break;
                    case 2:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->diffuseColor);
                        break;
                    case 3:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->backDiffuseColorDark);
                        break;
                    default:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->backDiffuseColorLight);
                        break;
                }

                if (displayManager->dmTransparency)
                    glEnable(GL_BLEND);
            }

            if (vars->first == 0) glEnd();
            glBegin(GL_POLYGON);

            glNormal3dv(e->Normal().coordinates);

            vars->first = 0;
            break;
        }
        case BRLCAD::VectorList::Element::TriangleStart: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleStart *> (element);
            if (displayManager->dmLight && vars->mFlag) {
                vars->mFlag = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, displayManager->ambientColor);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, displayManager->specularColor);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, displayManager->diffuseColor);

                switch (displayManager->dmLight) {
                    case 1:
                        break;
                    case 2:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->diffuseColor);
                        break;
                    case 3:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->backDiffuseColorDark);
                        break;
                    default:
                        glMaterialfv(GL_BACK, GL_DIFFUSE, displayManager->backDiffuseColorLight);
                        break;
                }

                if (displayManager->dmTransparency)
                    glEnable(GL_BLEND);
            }

            if (vars->first) {
                glBegin(GL_TRIANGLES);
            }

            glNormal3dv(e->Normal().coordinates);

            vars->first = 0;
            break;
        }
        case BRLCAD::VectorList::Element::PolygonMove: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonMove *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleMove: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleMove *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleDraw *> (element);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PolygonEnd: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonEnd *> (element);
            glVertex3dv(e->Point().coordinates);
            glEnd();
            vars->first = 1;
            break;
        }
        case BRLCAD::VectorList::Element::TriangleEnd: {
            break;
        }
        case BRLCAD::VectorList::Element::PolygonVertexNormal: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PolygonVertexNormal *> (element);
            glNormal3dv(e->Normal().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::TriangleVertexNormal: {
            auto *e = dynamic_cast<BRLCAD::VectorList::TriangleVertexNormal *> (element);
            glNormal3dv(e->Normal().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::PointDraw: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PointDraw *> (element);
            if (vars->first == 0) glEnd();
            vars->first = 0;
            glBegin(GL_POINTS);
            glVertex3dv(e->Point().coordinates);
            break;
        }
        case BRLCAD::VectorList::Element::LineWidth: {
            auto *e = dynamic_cast<BRLCAD::VectorList::LineWidth *> (element);
            auto lineWidth = static_cast<float>(e->Width());
            if (lineWidth > 0.0) {
                glLineWidth(lineWidth);
            }
            break;
        }
        case BRLCAD::VectorList::Element::PointSize: {
            auto *e = dynamic_cast<BRLCAD::VectorList::PointSize *> (element);
            auto pointSize = static_cast<float>(e->Size());
            if (pointSize > 0.0) {
                glPointSize(pointSize);
            }
            break;
        }
    }
    return true;
}

DisplayManager::DrawVListElementCallback::DrawVListElementCallback(const DisplayManager *displayManager,
                                                                   DrawVlistVars *vars) :
    displayManager(displayManager), vars(vars) {}

void DisplayManager::drawVList(BRLCAD::VectorList *vectorList)
{
    GLfloat originalPointSize, originalLineWidth;
    glGetFloatv(GL_POINT_SIZE, &originalPointSize);
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);

    DrawVListElementCallback::DrawVlistVars vars;
    DrawVListElementCallback drawVListElementCallback(this, &vars);
    vectorList->Iterate(drawVListElementCallback);
    

    if (vars.first == 0)
        glEnd();

    if (dmLight && dmTransparency)
        glDisable(GL_BLEND);

    glPointSize(originalPointSize);
    glLineWidth(originalLineWidth);
}


void DisplayManager::setFGColor(float r, float g, float b, float transparency) {
    wireColor[0] = r;
    wireColor[1] = g;
    wireColor[2] = b;
    wireColor[3] = transparency;

    diffuseColor[0] = wireColor[0] * 0.6f;
    diffuseColor[1] = wireColor[1] * 0.6f;
    diffuseColor[2] = wireColor[2] * 0.6f;
    diffuseColor[3] = wireColor[3];

    ambientColor[0] = wireColor[0] * 0.2f;
    ambientColor[1] = wireColor[1] * 0.2f;
    ambientColor[2] = wireColor[2] * 0.2f;
    ambientColor[3] = wireColor[3];

    specularColor[0] = ambientColor[0];
    specularColor[1] = ambientColor[1];
    specularColor[2] = ambientColor[2];
    specularColor[3] = ambientColor[3];

    backDiffuseColorDark[0] = wireColor[0] * 0.3f;
    backDiffuseColorDark[1] = wireColor[1] * 0.3f;
    backDiffuseColorDark[2] = wireColor[2] * 0.3f;
    backDiffuseColorDark[3] = wireColor[3];

    backDiffuseColorLight[0] = wireColor[0] * 0.9f;
    backDiffuseColorLight[1] = wireColor[1] * 0.9f;
    backDiffuseColorLight[2] = wireColor[2] * 0.9f;
    backDiffuseColorLight[3] = wireColor[3];

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseColor);
}

/*
 * Set the style of the line
 *
 * Valid Styles,
 * DM_SOLID_LINE=0
 * DM_DASHED_LINE=1
 */
void DisplayManager::setLineStyle(int style) {
    if (style == DM_DASHED_LINE) {
        glEnable(GL_LINE_STIPPLE);
    }
    else{
        glDisable(GL_LINE_STIPPLE);
    }
}

void DisplayManager::setLineWidth(int width)
{
    glLineWidth((GLfloat) width);
}

/*
 * Set the width and style of the line.
 *
 * Valid Styles,
 * DM_SOLID_LINE=0
 * DM_DASHED_LINE=1
 */
void DisplayManager::setLineAttr(int width, int style)
{
    if (width>0) {
        glLineWidth((GLfloat) width);
    }

    if (style == DM_DASHED_LINE) {
        glEnable(GL_LINE_STIPPLE);
    }
    else {
        glDisable(GL_LINE_STIPPLE);
    }
}

/*
 * Displays a saved display list identified by `list`
 */
void DisplayManager::drawDList(unsigned int list) {
    glCallList((GLuint) list);
}

/*
 * Generates `range` number of display lists and returns first display list's index
 */
unsigned int DisplayManager::genDLists(size_t range)
{
    return glGenLists((GLsizei)range);
}

/*
 * Supported subsequent opengl commands are compiled into the display list `list` rather than being rendered to the screen.
 * Should have called genDLists and generated the display list before calling this.
 */
void DisplayManager::beginDList(unsigned int list)
{
    glNewList((GLuint)list, GL_COMPILE);
}

/*
 * End of the display list initiated by beginDList.
 */
void DisplayManager::endDList()
{
    glEndList();
}

/*
 * End of the display list initiated by beginDList.
 */
GLboolean DisplayManager::isDListValid(unsigned int list)
{
    return glIsList(list);
}

void DisplayManager::freeDLists(unsigned int list, int range)
{
    glDeleteLists((GLuint)list, (GLsizei)range);
}
void DisplayManager::drawBegin()
{
    glClearColor(bgColor[0],bgColor[1],bgColor[2],1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
//    glMatrixMode(GL_MODELVIEW);
//    glPopMatrix();

}

void DisplayManager::saveState(){
    glPushAttrib(GL_ALL_ATTRIB_BITS);
}
void DisplayManager::restoreState(){
    glPopAttrib();
}

void DisplayManager::loadMatrix(const GLfloat *m)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(m);
}
void DisplayManager::loadPMatrix(const GLfloat *m)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(m);
}

