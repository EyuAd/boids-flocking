#include "Flock.h"

#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
constexpr float pi = 3.14159265358979323846f;
Flock flock;
bool paused = false;
bool debugView = false;
float fps = 0.0f;
int lastTimeMs = 0;
int fpsStartMs = 0;
int framesThisInterval = 0;

void drawText(float x, float y, const std::string& message) {
    glRasterPos2f(x, y);
    for (unsigned char character : message) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, character);
    }
}

std::string number(float value, int decimals = 1) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(decimals) << value;
    return stream.str();
}

void drawBoids() {
    const auto& boids = flock.boids();
    for (std::size_t i = 0; i < boids.size(); ++i) {
        const Boid& boid = boids[i];
        const float degrees = std::atan2(boid.velocity.y, boid.velocity.x) * 180.0f / pi;
        glPushMatrix();
        glTranslatef(boid.position.x, boid.position.y, 0.0f);
        glRotatef(degrees, 0.0f, 0.0f, 1.0f);

        if (debugView && i == 0) glColor3f(1.0f, 0.79f, 0.32f);
        else if (i % 3 == 0) glColor3f(0.35f, 0.85f, 1.0f);
        else if (i % 3 == 1) glColor3f(0.58f, 0.95f, 0.70f);
        else glColor3f(0.72f, 0.72f, 1.0f);

        // Local triangle points right; model rotation makes it face its velocity.
        glBegin(GL_TRIANGLES);
        glVertex2f(11.0f, 0.0f);
        glVertex2f(-7.0f, 5.0f);
        glVertex2f(-7.0f, -5.0f);
        glEnd();
        glPopMatrix();
    }
}

void drawDebug() {
    if (!debugView || flock.boids().empty()) return;
    const Boid& selected = flock.boids().front();
    const float radius = flock.settings().neighborRadius;
    glColor3f(1.0f, 0.70f, 0.25f);
    glBegin(GL_LINE_LOOP);
    for (int step = 0; step < 64; ++step) {
        const float angle = 2.0f * pi * static_cast<float>(step) / 64.0f;
        glVertex2f(selected.position.x + radius * std::cos(angle),
                   selected.position.y + radius * std::sin(angle));
    }
    glEnd();

    // Scale the arrow for readability; the boid's real speed is in the overlay.
    const Vec2 tip = selected.position + selected.velocity * 0.35f;
    const float velocityLength = std::sqrt(selected.velocity.x * selected.velocity.x +
                                           selected.velocity.y * selected.velocity.y);
    glBegin(GL_LINES);
    glVertex2f(selected.position.x, selected.position.y);
    glVertex2f(tip.x, tip.y);
    if (velocityLength > 0.0001f) {
        const Vec2 direction = selected.velocity / velocityLength;
        const Vec2 side{-direction.y, direction.x};
        const Vec2 base = tip - direction * 8.0f;
        glVertex2f(tip.x, tip.y);
        glVertex2f(base.x + side.x * 4.0f, base.y + side.y * 4.0f);
        glVertex2f(tip.x, tip.y);
        glVertex2f(base.x - side.x * 4.0f, base.y - side.y * 4.0f);
    }
    glEnd();
}

void drawOverlay() {
    const FlockSettings& s = flock.settings();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.045f, 0.075f, 0.12f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 580.0f);
    glVertex2f(Flock::width, 580.0f);
    glVertex2f(Flock::width, Flock::height);
    glVertex2f(0.0f, Flock::height);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.92f, 0.96f, 1.0f);
    drawText(12.0f, 680.0f, "BOIDS  |  Count: " + std::to_string(flock.boids().size()) +
             "  |  FPS: " + number(fps, 0) + (paused ? "  |  PAUSED" : "  |  RUNNING"));
    drawText(12.0f, 663.0f,
             "Rules: [1] Separation " + std::string(s.separation ? "ON" : "OFF") +
             "  [2] Alignment " + (s.alignment ? "ON" : "OFF") +
             "  [3] Cohesion " + (s.cohesion ? "ON" : "OFF"));
    drawText(12.0f, 646.0f,
             "Radius: " + number(s.neighborRadius, 0) +
             "  |  Weights S/A/C: " + number(s.separationWeight) + "/" +
             number(s.alignmentWeight) + "/" + number(s.cohesionWeight) +
             "  |  Speed: " + number(s.maxSpeed, 0) +
             "  |  Force: " + number(s.maxForce, 0));
    drawText(12.0f, 629.0f, "Space pause  |  R reset  |  + / - boids  |  D debug  |  Esc exit");
    drawText(12.0f, 612.0f, "[ / ] radius  |  Q/A separation  |  W/S alignment  |  E/C cohesion weight");
    drawText(12.0f, 595.0f, "Up/Down speed  |  Right/Left force  |  Debug: gold boid, radius, velocity");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    drawBoids();
    drawDebug();
    drawOverlay();
    glutSwapBuffers(); // Show a finished back buffer instead of a partly drawn frame.
}

void reshape(int width, int height) {
    width = std::max(width, 1);
    height = std::max(height, 1);
    const float worldAspect = Flock::width / Flock::height;
    const float windowAspect = static_cast<float>(width) / static_cast<float>(height);
    int viewportWidth = width;
    int viewportHeight = height;
    if (windowAspect > worldAspect) viewportWidth = static_cast<int>(height * worldAspect);
    else viewportHeight = static_cast<int>(width / worldAspect);

    // Center a fixed-aspect viewport: resizing adds bars rather than stretching.
    glViewport((width - viewportWidth) / 2, (height - viewportHeight) / 2,
               viewportWidth, viewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Flock::width, 0.0, Flock::height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    FlockSettings& s = flock.settings();
    switch (key) {
    case ' ': paused = !paused; break;
    case 'r': case 'R': flock = Flock{}; paused = false; debugView = false; break;
    case '+': case '=': flock.add(10); break;
    case '-': case '_': flock.remove(10); break;
    case '1': s.separation = !s.separation; break;
    case '2': s.alignment = !s.alignment; break;
    case '3': s.cohesion = !s.cohesion; break;
    case 'd': case 'D': debugView = !debugView; break;
    case '[': s.neighborRadius = std::max(30.0f, s.neighborRadius - 5.0f); break;
    case ']': s.neighborRadius = std::min(180.0f, s.neighborRadius + 5.0f); break;
    case 'q': case 'Q': s.separationWeight = std::min(3.0f, s.separationWeight + 0.1f); break;
    case 'a': case 'A': s.separationWeight = std::max(0.0f, s.separationWeight - 0.1f); break;
    case 'w': case 'W': s.alignmentWeight = std::min(3.0f, s.alignmentWeight + 0.1f); break;
    case 's': case 'S': s.alignmentWeight = std::max(0.0f, s.alignmentWeight - 0.1f); break;
    case 'e': case 'E': s.cohesionWeight = std::min(3.0f, s.cohesionWeight + 0.1f); break;
    case 'c': case 'C': s.cohesionWeight = std::max(0.0f, s.cohesionWeight - 0.1f); break;
    case 27: glutLeaveMainLoop(); return;
    default: return;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int, int) {
    FlockSettings& s = flock.settings();
    switch (key) {
    case GLUT_KEY_UP: s.maxSpeed = std::min(250.0f, s.maxSpeed + 10.0f); break;
    case GLUT_KEY_DOWN: s.maxSpeed = std::max(40.0f, s.maxSpeed - 10.0f); break;
    case GLUT_KEY_RIGHT: s.maxForce = std::min(600.0f, s.maxForce + 20.0f); break;
    case GLUT_KEY_LEFT: s.maxForce = std::max(80.0f, s.maxForce - 20.0f); break;
    default: return;
    }
    glutPostRedisplay();
}

void tick(int) {
    const int nowMs = glutGet(GLUT_ELAPSED_TIME);
    const float dt = std::clamp((nowMs - lastTimeMs) / 1000.0f, 0.0f, 0.05f);
    lastTimeMs = nowMs;
    if (!paused) flock.update(dt);

    ++framesThisInterval;
    if (nowMs - fpsStartMs >= 500) {
        fps = 1000.0f * framesThisInterval / static_cast<float>(nowMs - fpsStartMs);
        fpsStartMs = nowMs;
        framesThisInterval = 0;
    }
    glutPostRedisplay();
    glutTimerFunc(16, tick, 0);
}
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("2D Boids Flocking - C++ / OpenGL / FreeGLUT");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glClearColor(0.025f, 0.045f, 0.075f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    lastTimeMs = glutGet(GLUT_ELAPSED_TIME);
    fpsStartMs = lastTimeMs;
    glutTimerFunc(16, tick, 0);
    glutMainLoop();
    return EXIT_SUCCESS;
}
