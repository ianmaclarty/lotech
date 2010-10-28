#include <stdlib.h>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

void RenderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

void SetupRC() {
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
}

void specialUp(int key, int x, int y) {
    printf("key = %d, x = %d, y = %d\n", key, x, y);
    glutLeaveGameMode();
    exit(0);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutGameModeString("1024×768:32@75");
    glutEnterGameMode();
    //glutCreateWindow("Simple");
    glutDisplayFunc(RenderScene);
    glutSpecialUpFunc(specialUp);
    SetupRC();
    glutMainLoop();
    return 0;
}
