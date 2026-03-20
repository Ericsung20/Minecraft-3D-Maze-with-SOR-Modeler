#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  

struct Point {
    float x, y, z;
};

// 전역 변수 선언
std::vector<Point> points;           // 원래 점들의 목록
std::vector<Point> rotatedPoints;    // 회전 후 점들의 목록
std::vector<std::array<int, 3>> polygonIndices; // 폴리곤 인덱스 저장

// 회전 설정 변수
int rotationAxis = 0;           // 회전 축
float rotationAngle = 0.0f;     // 회전 각도

// 디스플레이 상태 변수
bool showWireframe = true;    // 와이어프레임 표시 여부
bool showPointsOnly = false;  // 점만 표시 여부


//회전 후 와이어프레임 그리기 함수
void drawWireframe() {
    glColor3f(0.0f, 0.0f, 0.0f);  // 와이어프레임 색상: 검은색
    glBegin(GL_LINES);            // 선 그리기 시작

    for (const auto& tri : polygonIndices) {  // 모든 삼각형을 순회
        // 현재 삼각형의 세 점 가져오기
        const Point& p1 = rotatedPoints[tri[0]];
        const Point& p2 = rotatedPoints[tri[1]];
        const Point& p3 = rotatedPoints[tri[2]];

        // 특정 축의 최소/최대 점을 연결하는 변을 생략
        auto skipEdge = [&](const Point& a, const Point& b, int axis) {
            auto minMax = std::minmax_element(rotatedPoints.begin(), rotatedPoints.end(),
                [axis](const Point& p1, const Point& p2) {
                    if (axis == 0) return p1.x < p2.x;  // X축
                    if (axis == 1) return p1.y < p2.y;  // Y축
                    return p1.z < p2.z;                 // Z축
                });

            // 주어진 축의 최소/최대 점을 연결하는 변인지 확인
            if (axis == 0)  // X축 회전 시
                return (a.x == minMax.first->x && b.x == minMax.second->x) ||
                (b.x == minMax.first->x && a.x == minMax.second->x);
            if (axis == 1)  // Y축 회전 시
                return (a.y == minMax.first->y && b.y == minMax.second->y) ||
                (b.y == minMax.first->y && a.y == minMax.second->y);
            };

        // 각 변을 검사하고 그리지 않을 변은 생략
        if (!skipEdge(p1, p2, rotationAxis)) {
            glVertex3f(p1.x, p1.y, p1.z); glVertex3f(p2.x, p2.y, p2.z);
        }
        if (!skipEdge(p2, p3, rotationAxis)) {
            glVertex3f(p2.x, p2.y, p2.z); glVertex3f(p3.x, p3.y, p3.z);
        }
    }
    glEnd();  // 선 그리기 종료
}


// 메뉴 선택 핸들러
void handleMenu(int option) {
    switch (option) {
    case 1:  // 와이어프레임만 보이기
        showWireframe = true;
        showPointsOnly = false;
        break;
    case 2:  // 버텍스만 보이기
        showWireframe = false;
        showPointsOnly = true;
        break;

    case 3:  // Reset 초기화
        points.clear();        
        rotatedPoints.clear();  
        polygonIndices.clear(); 

        // 상태 및 회전 설정 초기화
        showWireframe = true;
        showPointsOnly = false;
        rotationAxis = 0;       // 회전 축 초기화
        rotationAngle = 0.0f;   // 회전 각도 초기화

        std::cout << "데이터가 초기화되었습니다. 새로 점을 찍어주세요.\n";
        glutPostRedisplay();  // 화면 새로고침
        break;

    case 4:  // Start Maze 실행
        std::cout << "미로 게임 시작: CG_Final_Maze.exe 실행 중...\n";
        system("CG_Final_Maze.exe");
        break;
    }
    glutPostRedisplay();  // 화면 새로고침
}


// 우클릭 메뉴 생성
void createMenu() {
    glutCreateMenu(handleMenu);  // 메뉴 생성
    glutAddMenuEntry("Wireframe", 1);       // 와이어프레임 표시
    glutAddMenuEntry("Only vertex", 2);     // 점만 표시
    glutAddMenuEntry("Reset", 3);           // 초기 상태로 초기화
    glutAddMenuEntry("Start Maze", 4);      // Start Maze 항목
    glutAttachMenu(GLUT_RIGHT_BUTTON);      // 우클릭 메뉴 연결
}


// 디스플레이 함수
void display() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);    // 배경색 설정: 회색 
    glClear(GL_COLOR_BUFFER_BIT);   // 컬러 버퍼 지우기

    // 가로 및 세로 기준선 그리기
    glColor3f(0.0f, 0.0f, 0.0f);  // 검은색으로 설정
    glLineWidth(1.0f);           
    glBegin(GL_LINES);      

    // 가로선 그리기 (X축)
    glVertex2f(-1.0f, 0.0f);  // 왼쪽 끝점
    glVertex2f(1.0f, 0.0f);   // 오른쪽 끝점

    // 세로선 그리기 (Y축)
    glVertex2f(0.0f, -1.0f);  // 아래쪽 끝점
    glVertex2f(0.0f, 1.0f);   // 위쪽 끝점
    glEnd();  

    // 점들 그리기
    glPointSize(3.0f);      // 점 크기 설정
    glColor3f(1.0f, 0.0f, 0.0f);  // 빨간색 설정
    glBegin(GL_POINTS);   

    for (const auto& point : points) {  
        glVertex3f(point.x, point.y, point.z);  
    }
    glEnd(); 

    // 회전된 점들 그리기 
    glPointSize(3.0f);
    glColor3f(1.0f, 0.0f, 0.0f); 
    glBegin(GL_POINTS);  // 점 그리기 시작

    for (const auto& point : rotatedPoints) {  // 회전된 점들 순회
        glVertex3f(point.x, point.y, point.z);  // 회전된 점 출력
    }
    glEnd();  

    // Only Vertex 모드
        if (showPointsOnly) {
            glPointSize(5.0f);
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POINTS);
            for (const auto& point : points) {
                glVertex3f(point.x, point.y, point.z);  // 원래 점
            }
            for (const auto& point : rotatedPoints) {
                glVertex3f(point.x, point.y, point.z);  // 회전된 점
            }
            glEnd();
        }

    // 와이어프레임 표시
    if (showWireframe && !showPointsOnly) {
        drawWireframe();
    }

    glFlush(); 
}


// 마우스 클릭 시 점 추가
void mouse(int button, int state, int x, int y) {
    // 마우스 좌클릭 눌렸을 때 처리
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float nx = (x / (float)glutGet(GLUT_WINDOW_WIDTH)) * 2.0f - 1.0f;
        float ny = 1.0f - (y / (float)glutGet(GLUT_WINDOW_HEIGHT)) * 2.0f;

        // 변환된 좌표를 points 벡터에 추가
        points.push_back({ nx, ny, 0.0f });

        // 점 좌표 콘솔 창에 출력
        std::cout << "점 추가됨: (" << std::fixed << std::setprecision(6) << nx << ", " << ny << ", 0.0)" << std::endl;

        // 점이 추가되었으므로 화면을 새로 그리도록 요청
        glutPostRedisplay();
    }
}


// 회전 로직 수행
void rotatePoints() {
    rotatedPoints.clear();  // 이전 회전된 점 초기화
    polygonIndices.clear(); // 이전 폴리곤 인덱스 초기화

    // 회전 각도를 라디안으로 변환
    float radians = rotationAngle * M_PI / 180.0f;

    // 회전 단계 개수 계산
    int steps = 0;
    if (rotationAngle > 0.0f) {
        steps = 360 / static_cast<int>(rotationAngle);
    }

    // 원래 점의 개수
    int P = (int)points.size();

    // 각 회전 단계의 점들을 저장할 레이어 생성
    std::vector<std::vector<Point>> layers;
    layers.reserve(steps + 1);  

    // 각 회전 단계마다 회전 변환 수행
    for (int i = 0; i <= steps; ++i) {
        float currentAngle = i * radians;  // 현재 회전 각도 계산

        // 현재 회전 단계에서의 점들을 저장할 벡터
        std::vector<Point> layer;
        layer.reserve(P);  

        // 원래 점들을 순회하며 회전 변환 수행
        for (const auto& point : points) {
            Point rotated = point;  // 원래 점 복사

            // 회전 축에 따라 다른 회전 변환 수행
            if (rotationAxis == 0) {  // X축 회전
                rotated.y = point.y * cos(currentAngle) - point.z * sin(currentAngle);
                rotated.z = point.y * sin(currentAngle) + point.z * cos(currentAngle);
            }
            else if (rotationAxis == 1) {  // Y축 회전
                rotated.x = point.x * cos(currentAngle) + point.z * sin(currentAngle);
                rotated.z = -point.x * sin(currentAngle) + point.z * cos(currentAngle);
            }
            else if (rotationAxis == 2) {  // Z축 회전
                rotated.x = point.x * cos(currentAngle) - point.y * sin(currentAngle);
                rotated.y = point.x * sin(currentAngle) + point.y * cos(currentAngle);
            }

            // 회전된 점을 현재 레이어에 추가
            layer.push_back(rotated);
        }
        // 현재 레이어를 레이어 리스트에 추가
        layers.push_back(layer);
    }

    // 모든 회전된 점을 평면 벡터로 정리하여 rotatedPoints에 추가
    rotatedPoints.reserve((steps + 1) * P);  
    for (int i = 0; i <= steps; ++i) {
        for (int j = 0; j < P; ++j) {
            rotatedPoints.push_back(layers[i][j]);
        }
    }

    // 폴리곤 인덱스 생성
    for (int i = 0; i < steps; ++i) {
        for (int j = 0; j < P; ++j) {
            // 현재 레이어와 다음 레이어의 점 인덱스 계산
            int currLayerCurrPoint = i * P + j;               // 현재 레이어 현재 점
            int nextLayerCurrPoint = (i + 1) * P + j;         // 다음 레이어 현재 점
            int nextLayerNextPoint = (i + 1) * P + ((j + 1) % P);  // 다음 레이어 다음 점
            int currLayerNextPoint = i * P + ((j + 1) % P);        // 현재 레이어 다음 점

            // 첫 번째 삼각형 추가
            polygonIndices.push_back({ currLayerCurrPoint, nextLayerCurrPoint, nextLayerNextPoint });

            // 두 번째 삼각형 추가
            polygonIndices.push_back({ currLayerCurrPoint, nextLayerNextPoint, currLayerNextPoint });
        }
    }
}


// 회전 입력 받는 함수
void handleRotationInput() {
    // 사용자에게 회전 축 선택 요청
    std::cout << "회전 축을 선택하세요 (0 = X축, 1 = Y축, 2 = Z축): ";
    std::cin >> rotationAxis;  // 회전 축 입력 받기

    // 회전 축이 잘못된 경우 재입력 요청
    while (rotationAxis < 0 || rotationAxis > 2) {
        std::cout << "잘못된 축입니다. 0, 1 또는 2를 입력하세요: ";
        std::cin >> rotationAxis;  
    }

    // 회전 각도 입력 요청
    std::cout << "회전 각도를 입력하세요 (360의 약수여야 합니다): ";
    std::cin >> rotationAngle;  // 회전 각도 입력 받기

    // 유효하지 않은 각도가 입력되면 재입력 요청
    while ((int)rotationAngle <= 0 || 360 % (int)rotationAngle != 0) {
        std::cout << "잘못된 각도입니다. 360의 약수를 입력하세요: ";
        std::cin >> rotationAngle;  
    }

    // 점 회전 수행
    rotatePoints();  

    // 화면 새로고침 요청
    glutPostRedisplay();
}


// 점과 폴리곤 정보를.dat 파일로 저장
void savePointsToDAT() {
    const std::string fileName = "model.dat";  // 파일 이름을 "model.dat"으로 고정

    // 파일 열기 
    std::ofstream file(fileName);

    // 파일 열기에 실패했는지 확인
    if (!file) {
        std::cerr << "파일 열기에 실패했습니다. 쓰기 권한을 확인하세요." << std::endl;
        return;
    }

    // 회전된 정점 개수와 폴리곤 면 개수 계산
    int vertexCount = static_cast<int>(rotatedPoints.size());   // 정점 개수
    int faceCount = static_cast<int>(polygonIndices.size());    // 면 개수

    // 정점 정보 기록
    file << "VERTEX = " << vertexCount << "\n";  // 정점 수 출력
    for (const auto& v : rotatedPoints) {
        // 소수점 6자리 정점 좌표 기록
        file << std::fixed << std::setprecision(6)
            << v.x << "\t" << v.y << "\t" << v.z << "\n";
    }

    // 폴리곤 면 정보 기록
    file << "FACE = " << faceCount << "\n";  // 면 개수 출력

    for (const auto& tri : polygonIndices) {
        // 폴리곤 면을 구성하는 정점 인덱스를 파일에 기록
        file << tri[0] << "\t" << tri[1] << "\t" << tri[2] << "\n";
    }

    // 파일 닫기
    file.close();

    // 사용자에게 저장 성공 메시지 출력
    std::cout << "정점과 폴리곤 정보가 파일 '" << fileName << "'에 저장되었습니다." << std::endl;
}


// 키보드 입력 처리 함수 
void keyboard(unsigned char key, int x, int y) {
    // Enter 키가 눌렸을 경우
    if (key == 13) {
        handleRotationInput();  // 회전 축 및 각도 입력 함수 호출
    }
    // 's' 키가 눌렸을 경우
    else if (key == 's' || key == 'S') {  // 대소문자 구분 없이 처리
        savePointsToDAT();  // 현재 회전된 점과 폴리곤 데이터를 .dat 파일로 저장
    }
    // 'Backspace' 키가 눌렸을 경우
    else if (key == 8) {  // Backspace 키
        if (!points.empty()) {
            points.pop_back();  // 가장 최근에 추가된 점 제거  
            std::cout << "가장 최근의 점이 제거되었습니다.\n";

            if (rotationAngle > 0.0f) {  // 회전이 설정되어 있다면
                rotatePoints();  // 회전된 점들과 폴리곤 인덱스 재생성
            }
            else {  // 회전이 설정되어 있지 않다면
                rotatedPoints.clear();
                polygonIndices.clear();
            }

            glutPostRedisplay();  // 화면 새로고침
        }
        else {
            std::cout << "제거할 점이 없습니다.\n";
        }
    }
}


// 초기화 함수
void init() {
    // OpenGL 행렬 모드를 '투영 행렬 모드'로 설정
    glMatrixMode(GL_PROJECTION);

    // 현재 투영 행렬을 단위 행렬로 초기화
    glLoadIdentity();

    // 2D 직교 투영 설정
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}


// 메인 함수
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);    // 깊이 버퍼 추가
    glutInitWindowSize(800, 600);             // 창 크기 설정
    glutCreateWindow("OpenGL SOR Modeler");   // 창 생성 및 제목 설정
    init();
    // 콜백 함수 등록
    glutDisplayFunc(display);    // 디스플레이 콜백 함수 등록
    glutMouseFunc(mouse);        // 마우스 콜백 함수 등록
    glutKeyboardFunc(keyboard);  // 키보드 콜백 함수 등록

    // 우클릭 메뉴 생성
    createMenu();

    // 사용자 안내 메시지 출력
    std::cout << "마우스를 사용하여 점을 추가하세요.\n";
    std::cout << "'Backspace' 키를 눌러 최근 vertex를 지울 수 있습니다.\n";
    std::cout << "Enter 키를 눌러 회전 설정을 구성하세요.\n";
    std::cout << "우클릭 메뉴를 사용해 보세요.\n";
    std::cout << "'S' 키를 눌러 모델을 .dat 파일로 저장하세요.\n";
    

    // 메인 루프 시작
    glutMainLoop();
    return 0;
}