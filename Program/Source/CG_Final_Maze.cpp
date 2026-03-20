#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#define M_PI 3.14159265358979323846

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


// 모델의 정점을 나타내는 구조체
struct ModelVertex {
    float x, y, z;  // 정점의 좌표
};

// 모델의 면을 나타내는 구조체
struct ModelFace {
    int v1, v2, v3;  // 삼각형을 구성하는 정점의 인덱스
};

// 모델의 위치 및 상태를 나타내는 구조체
struct Model {
    float x, y, z;  // 모델 위치
    bool collected; // 모델 수집 여부
    float phase;    // 플로팅 모션의 위상
};


// 모델 데이터를 저장하는 벡터
std::vector<ModelVertex> modelVertices; // 모델의 정점 리스트
std::vector<ModelFace> modelFaces;      // 모델의 면 리스트

// 모델 인스턴스 리스트 및 수집 개수
std::vector<Model> models;              // 생성된 모델 리스트
int collectedCount = 0;                 // 수집된 모델 개수

// 미로 크기 및 데이터 정의
const int mazeWidth = 16;
const int mazeHeight = 14;
int maze[mazeHeight][mazeWidth] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 4, 1, 0, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1},
    {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1},
    {0, 0, 0, 2, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 3},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 1, 1, 1, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 2, 1 ,1},
    {1, 0, 0, 0, 2, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
    {1, 4, 1, 1, 1, 2, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

// 카메라 위치 및 방향 정보
float cameraX = 0.0f, cameraY = 0.5f, cameraZ = 6.0f; // 카메라 초기 위치
float cameraYaw = 0.0f; // 카메라 회전 각도

// 카메라 이동 속도
const float moveSpeed = 0.06f;  // 이동 속도
const float turnSpeed = 1.7f;   // 회전 속도

// 입력 및 게임 상태 관리 변수
bool keys[256];               // 키 입력 상태
bool inPauseMode = false;     // 일시정지 상태 여부
bool inputEnabled = false;    // 입력 활성화 여부
bool gameFinished = false;    // 게임 종료 여부
bool escapeTriggered = false; // 탈출 트리거 여부
int pauseTimer = 0;           // 일시정지 타이머
int messageState = 0;         // 현재 메시지 단계
int startDelay = 90;          // 메시지 표시 지연 시간
int escapeDelay = 0;          // 탈출 메시지 지연 시간

// 텍스처 관리 변수
GLuint wallTexture;
GLuint groundTexture;
GLuint lavaTexture;
GLuint GoalTexture;
GLuint outerWallTexture;

// 수집 메시지 상태 변수
bool showCollectedMessage = false;
int collectedMessageTimer = 0;

// "이런!" 메시지 상태 변수
bool showOopsMessage = false;
int oopsMessageTimer = 0;

// 목표 도착 메시지 상태 변수
bool showEscapeMessage = false;
int escapeMessageTimer = 0;

// 카메라 초기 위치와 회전 각도 저장
float startX = 0.0f;
float startY = 0.5f;
float startZ = 6.0f;
float startYaw = 0.0f;

// 함수 프로토타입
bool checkCollision(float testX, float testZ);
bool checkModelCollision(float camX, float camZ, float modelX, float modelZ, float collisionDistance = 0.5f);
void checkModelCollection();

// 배경음악 재생
void playBackgroundMusic() {
    if (mciSendString(TEXT("open \"background.wav\" type mpegvideo alias bgm"), NULL, 0, NULL) != 0) {
        MessageBox(NULL, TEXT("음악 파일 열기 실패!"), TEXT("오류"), MB_ICONERROR | MB_OK);
    }
    if (mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL) != 0) {
        MessageBox(NULL, TEXT("배경음악 재생 실패!"), TEXT("오류"), MB_ICONERROR | MB_OK);
    }
}

// 배경음악 정지
void stopBackgroundMusic() {
    if (mciSendString(TEXT("stop bgm"), NULL, 0, NULL) != 0) {
        MessageBox(NULL, TEXT("배경음악 정지 실패!"), TEXT("오류"), MB_ICONERROR | MB_OK);
    }
    if (mciSendString(TEXT("close bgm"), NULL, 0, NULL) != 0) {
        MessageBox(NULL, TEXT("음악 장치 닫기 실패!"), TEXT("오류"), MB_ICONERROR | MB_OK);
    }
}

// 아이템 획득 효과음 재생
void playCollectSound() {
    PlaySound(TEXT("item_get_sound.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 용암 효과음 재생
void playWarningSound() {
    PlaySound(TEXT("lava_fall.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 탈출 성공 효과음
void playSuccessSound() {
    PlaySound(TEXT("success.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}


// 텍스처 로드 함수
GLuint LoadTexture(const char* filename) {
    GLuint texture;           // 텍스처 객체 ID 저장 변수
    int width, height;        // 이미지의 너비와 높이 저장 변수
    unsigned char* data;      // 이미지 데이터 저장을 위한 포인터

    // BMP 파일 열기 시도
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        // 파일 열기에 실패한 경우 오류 메시지 출력
        printf("파일을 열 수 없습니다: '%s'\n", filename);
        return 0;  // 오류 발생 시 0 반환
    }

    // BMP 파일의 헤더(54 바이트) 읽기
    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    // BMP 헤더에서 이미지의 너비(width)와 높이(height) 추출
    width = *(int*)&header[18];
    height = *(int*)&header[22];

    // 이미지 데이터 크기 계산
    int size = 3 * width * height;

    // 이미지 데이터를 저장할 메모리 동적 할당
    data = (unsigned char*)malloc(size);

    // BMP 파일에서 실제 이미지 데이터 읽기
    fread(data, sizeof(unsigned char), size, file);
    fclose(file);  // 파일 닫기

    // BMP 데이터는 BGR 형식으로 저장되어 있으므로 RGB로 변환
    for (int i = 0; i < size; i += 3) {
        unsigned char tmp = data[i];
        data[i] = data[i + 2];          // Blue <-> Red 교환
        data[i + 2] = tmp;
    }

    // OpenGL에서 텍스처 객체 생성
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 텍스처 필터링 옵션 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 텍스처 이미지를 OpenGL에 업로드
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // 동적으로 할당한 메모리 해제
    free(data);
    return texture;
}


// 조명 초기화 함수
void initLighting() {
    glEnable(GL_LIGHTING);  // OpenGL 조명 활성화
    glEnable(GL_LIGHT0);    // 기본 광원(LIGHT0) 활성화

    // 광원 속성 설정
    GLfloat lightPos[] = { 10.0f, 10.0f, 10.0f, 1.0f };     // 광원의 위치 (x, y, z, w)
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };    // 주변광 (Ambient Light) 색상 - 희미한 기본 조명
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };    // 확산광 (Diffuse Light) 색상 - 물체 표면 색상
    GLfloat lightSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };   // 반사광 (Specular Light) 색상 - 빛 반사 효과


    // 광원의 속성 적용
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);       // 광원의 위치 설정
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);    // 주변광 색상 설정
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);    // 확산광 색상 설정
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);  // 반사광 색상 설정

    // 재질 속성 설정
    GLfloat matAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };   // 재질의 주변광 반사 정도
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };   // 재질의 확산광 반사 정도
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // 재질의 반사광 반사 정도
    GLfloat matShininess[] = { 80.0f };                  // 재질의 반짝임 정도 

    // 재질 속성 적용
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);     // 앞/뒤면 주변광 반사 설정
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);     // 앞/뒤면 확산광 반사 설정
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);   // 앞/뒤면 반사광 설정
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess); // 반짝임 정도 설정

    // 컬러 재질 활성화 및 음영 모델 설정
    glEnable(GL_COLOR_MATERIAL);  // 색상 재질 활성화 
    glShadeModel(GL_SMOOTH);      // 음영 처리를 Smooth Shading으로 설정 
}


// 초기화 함수
void init() {
    // 기본 OpenGL 기능 활성화
    glEnable(GL_DEPTH_TEST);  // 깊이 테스트 활성화 
    glEnable(GL_TEXTURE_2D);  // 텍스처 맵핑 활성화

    // 배경 색상 설정
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);  // 회색 배경 설정

    // 텍스처 로드 및 설정
    wallTexture = LoadTexture("wall_grassbrick.bmp");  // 벽 텍스처 로드
    groundTexture = LoadTexture("ground_grass.bmp");   // 땅 텍스처 로드
    lavaTexture = LoadTexture("ground_lava.bmp");      // 용암 텍스처 로드
    GoalTexture = LoadTexture("Goal_ORE.bmp");         // 목표 지점 텍스처 로드
    outerWallTexture = LoadTexture("sky.bmp");         // 외부 벽 텍스처 로드

    // 조명 초기화
    initLighting();  // 광원과 재질 설정

    playBackgroundMusic();
}


// DAT 파일에서 3D 모델 데이터 로드하는 함수
bool loadModelFromDAT(const char* filename) {
    std::ifstream file(filename);  // 파일 열기
    if (!file.is_open()) {         // 파일 열기 실패 시
        std::cerr << "DAT 파일 열기 실패: " << filename << std::endl;  // 오류 메시지 출력
        return false;
    }

    // 기존 모델 데이터 초기화
    modelVertices.clear();  // 정점 리스트 초기화
    modelFaces.clear();     // 면 리스트 초기화

    // 변수 선언 및 초기화
    std::string line;
    int vertexCount = 0;           // 정점 개수
    int faceCount = 0;             // 면 개수
    bool readingVertices = false;  // 현재 정점 데이터를 읽고 있는지 여부
    bool readingFaces = false;     // 현재 면 데이터를 읽고 있는지 여부

    // 파일 전체 줄 읽기 루프
    while (std::getline(file, line)) {

        // 정점 데이터 읽기 시작
        if (line.rfind("VERTEX =", 0) == 0) {  // 줄의 시작이 "VERTEX ="인지 확인
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> dummy >> vertexCount;
            readingVertices = true;         // 정점 데이터 읽기 시작
            readingFaces = false;           // 면 데이터 읽기 중지
            continue;
        }

        // 면 데이터 읽기 시작 
        else if (line.rfind("FACE =", 0) == 0) {  // 줄의 시작이 "FACE ="인지 확인
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> dummy >> faceCount;
            readingVertices = false;    // 정점 데이터 읽기 중지
            readingFaces = true;        // 면 데이터 읽기 시작
            continue;  // 다음 줄로 이동
        }

        // 정점 데이터를 읽는 경우
        if (readingVertices) {
            float vx, vy, vz;       // 정점의 x, y, z 좌표
            std::istringstream iss(line);

            if (iss >> vx >> vy >> vz) {
                ModelVertex v{ vx, vy, vz };  // 정점 구조체 생성
                modelVertices.push_back(v);   // 정점을 정점 리스트에 추가

                // 모든 정점을 읽었으면 정점 읽기 종료
                if ((int)modelVertices.size() == vertexCount) {
                    readingVertices = false;
                }
            }
        }

        // 면 데이터를 읽는 경우
        else if (readingFaces) {
            int f1, f2, f3;          // 삼각형 면을 구성하는 정점 인덱스
            std::istringstream iss(line);

            if (iss >> f1 >> f2 >> f3) {
                ModelFace f{ f1, f2, f3 };  // 면 구조체 생성
                modelFaces.push_back(f);    // 면을 면 리스트에 추가

                // 모든 면 데이터를 읽었으면 면 읽기 종료
                if ((int)modelFaces.size() == faceCount) {
                    readingFaces = false;
                }
            }
        }
    }

    // 파일 읽기 완료 후 파일 닫기
    file.close();

    // 로드 결과 출력
    std::cout << "모델 로드 완료: " << vertexCount << "개의 정점, " << faceCount << "개의 면." << std::endl;
    return true;
}


// 삼각형의 법선 벡터를 계산하는 함수
void computeNormal(const ModelVertex& v1, const ModelVertex& v2, const ModelVertex& v3, float& nx, float& ny, float& nz) {
    // v1, v2, v3: 삼각형을 구성하는 세 개의 정점
    // 첫 번째 벡터(u) 계산: v1에서 v2로 향하는 벡터
    float ux = v2.x - v1.x;
    float uy = v2.y - v1.y;
    float uz = v2.z - v1.z;

    // 두 번째 벡터(v) 계산: v1에서 v3로 향하는 벡터
    float vx = v3.x - v1.x;
    float vy = v3.y - v1.y;
    float vz = v3.z - v1.z;

    // 벡터 u와 v의 외적(크로스 프로덕트) 계산
    nx = uy * vz - uz * vy;
    ny = uz * vx - ux * vz;
    nz = ux * vy - uy * vx;

    // 법선 벡터 정규화
    float length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 0.0f) {
        nx /= length;
        ny /= length;
        nz /= length;
    }
}


// 모델 초기화 함수
void initializeModels() {
    models.clear();  // 기존 모델 리스트 초기화

    // 미로 전체를 순회하며 모델 위치 설정
    for (int z = 0; z < mazeHeight; ++z) {   // z축 방향(미로의 세로)
        for (int x = 0; x < mazeWidth; ++x) {  // x축 방향(미로의 가로)

            // 미로 배열 값이 '4'인 위치에 모델 추가
            if (maze[z][x] == 4) {
                Model m;
                m.x = (float)x;     // x축 좌표 설정
                m.y = 0.5f;         // y축 좌표 설정 
                m.z = (float)z;     // z축 좌표 설정
                m.collected = false;  // 수집 여부 초기화 

                m.phase = (float)(x + z) * 0.5f; // 예: 위치에 따라 위상 차이

                models.push_back(m);

                // 디버깅 메시지 출력 (모델 초기화 위치)
                printf("Model initialized at (%f, %f, %f)\n", m.x, m.y, m.z);
            }
        }
    }

    // 총 모델 수 출력
    printf("Total models initialized: %d\n", (int)models.size());
}


// 벽 그리기 함수
void drawCube(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z); // 큐브를 그릴 위치로 변환

    glBegin(GL_QUADS); // 사각형(QUADS) 그리기 시작

    // 앞면
    glTexCoord2f(0.05f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.05f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.05f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.05f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // 뒷면
    glTexCoord2f(0.05f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.05f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.05f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.05f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

    // 왼쪽면
    glTexCoord2f(0.05f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.05f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.05f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.05f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // 오른쪽면
    glTexCoord2f(0.05f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.05f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.05f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.05f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);

    // 윗면
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // 밑면
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

    glEnd();     // 사각형 그리기 종료
    glPopMatrix();
}


// 땅 그리기 함수
void drawGround() {

    // 미로의 높이와 너비를 순회하며 각각의 셀에 대해 처리
    for (int z = 0; z < mazeHeight; ++z) { // z축 방향 순회 
        for (int x = -4; x < mazeWidth; ++x) { // x축 방향 순회 

            // 셀의 값에 따라 다른 텍스처를 바인딩
            if (maze[z][x] == 2) {
                glBindTexture(GL_TEXTURE_2D, lavaTexture); // 셀 값이 2면 용암 텍스처 적용
            }
            else if (maze[z][x] == 3) {
                glBindTexture(GL_TEXTURE_2D, GoalTexture); // 셀 값이 3이면 목표 텍스처 적용
            }
            else if (maze[z][x] == 4) {
                glBindTexture(GL_TEXTURE_2D, groundTexture); // 셀 값이 4이면 일반 땅 텍스처 사용
            }
            else {
                glBindTexture(GL_TEXTURE_2D, groundTexture); // 일반 땅 텍스처
            }

            // 텍스처 매핑을 위한 오프셋
            float offsetX = 0.0f;
            float offsetZ = 0.0f;

            // 사각형 그리기 시작
            glBegin(GL_QUADS);

            // 텍스처 좌표와 월드 좌표를 설정
            glTexCoord2f(0.1f, 0.0f); glVertex3f(x - 0.5f + offsetX, 0.0f, z - 0.5f + offsetZ); // 왼쪽 아래
            glTexCoord2f(1.1f, 0.0f); glVertex3f(x + 0.5f + offsetX, 0.0f, z - 0.5f + offsetZ); // 오른쪽 아래
            glTexCoord2f(1.1f, 1.0f); glVertex3f(x + 0.5f + offsetX, 0.0f, z + 0.5f + offsetZ); // 오른쪽 위
            glTexCoord2f(0.1f, 1.0f); glVertex3f(x - 0.5f + offsetX, 0.0f, z + 0.5f + offsetZ); // 왼쪽 위

            glEnd(); // 사각형 그리기 종료
        }
    }
}


// 미로 그리기 함수
void drawMaze() {

    // 미로의 모든 셀을 순회
    for (int z = 0; z < mazeHeight; ++z) { // z축 방향 순회
        for (int x = 0; x < mazeWidth; ++x) { // x축 방향 순회

            // 벽인 경우(maze[z][x] == 1)
            if (maze[z][x] == 1) {
                glBindTexture(GL_TEXTURE_2D, wallTexture);
                drawCube(x, 0.5f, z);   // 벽 위치에 큐브를 그리기
            }
            // 목표 지점(maze[z][x] == 3)인 경우
            else if (maze[z][x] == 3) {
                glPushMatrix();
                glTranslatef(x, 0.5f, z);   // 목표 지점 위치로 이동

                glBindTexture(GL_TEXTURE_2D, GoalTexture); // 목표 지점 텍스처 사용

                glBegin(GL_QUADS);  // 사각형(QUADS) 그리기 시작

                // 목표 지점의 오른쪽 면만 그리기
                glTexCoord2f(0.09f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); // 아래 앞
                glTexCoord2f(1.09f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); // 아래 뒤
                glTexCoord2f(1.09f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f); // 위 뒤
                glTexCoord2f(0.09f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f); // 위 앞

                glEnd();    // 사각형(QUADS) 그리기 종료

                glPopMatrix();
            }
        }
    }
}


// 외부 벽 그리기 함수
void drawOuterWalls() {
    glBindTexture(GL_TEXTURE_2D, outerWallTexture); // 외부 벽 텍스처 바인딩

    // 전면 벽 (앞쪽)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(16.0f, 0.0f, -2.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(16.0f, 8.0f, -2.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 8.0f, -2.0f);
    glEnd();

    // 후면 벽 (뒤쪽)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.0f, 16.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(16.0f, 0.0f, 16.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(16.0f, 8.0f, 16.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 8.0f, 16.0f);
    glEnd();

    // 좌측 벽 (왼쪽)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-3.0f, 0.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-3.0f, 0.0f, 16.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-3.0f, 8.0f, 16.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-3.0f, 8.0f, -2.0f);
    glEnd();

    // 우측 벽 (오른쪽)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(16.0f, 0.0f, -2.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(16.0f, 0.0f, 16.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(16.0f, 8.0f, 16.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(16.0f, 8.0f, -2.0f);
    glEnd();
}


// 모델 그리기 함수
void drawModel(float offsetX, float offsetY, float offsetZ, float phase) {
    if (modelVertices.empty() || modelFaces.empty()) return;    // 모델 데이터가 비어 있으면 함수 종료

    glPushMatrix();

    float elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // 경과 시간 (초)
    float floatAmplitude = 0.07f; // 플로팅 진폭
    float floatFrequency = 1.0f; // 플로팅 주파수 (Hz)
    float floatingY = offsetY + floatAmplitude * sin(2 * M_PI * floatFrequency * elapsedTime + phase); // y 오프셋 계산

    glTranslatef(offsetX, floatingY, offsetZ);
    glScalef(0.4f, 0.4f, 0.4f);    // 모델 크기 축소 (0.4배 크기로 조정)

    // 재질(Material) 설정
    GLfloat matAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matShininess[] = { 50.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    glColor3f(1.0f, 1.0f, 1.0f);    // 기본 색상

    // 면마다 법선 벡터를 계산
    glBegin(GL_TRIANGLES);  // 삼각형 단위로 그리기 시작
    float nx, ny, nz;       // 법선 벡터 저장용 변수
    float minY = modelVertices[0].y, maxY = modelVertices[0].y;
    for (const auto& vertex : modelVertices) {
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.y > maxY) maxY = vertex.y;
    }

    // 각 면(face)에 대해 삼각형을 렌더링
    for (const auto& f : modelFaces) {
        const ModelVertex& v1 = modelVertices[f.v1]; // 첫 번째 정점
        const ModelVertex& v2 = modelVertices[f.v2]; // 두 번째 정점
        const ModelVertex& v3 = modelVertices[f.v3]; // 세 번째 정점

        // y축 최소값과 최대값 포함된 삼각형은 스킵
        if ((v1.y == minY || v1.y == maxY) &&
            (v2.y == minY || v2.y == maxY) &&
            (v3.y == minY || v3.y == maxY)) {
            continue;
        }

        // 현재 면의 법선 벡터 계산
        computeNormal(v1, v2, v3, nx, ny, nz);
        glNormal3f(nx, ny, nz);

        // 삼각형의 세 정점을 렌더링
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();    // 삼각형 그리기 종료

    glPopMatrix();
}


// 모델을 동적으로 그리는 함수
void drawModels() {
    for (const auto& model : models) {

        // 수집되지 않은 모델만 그리기
        if (!model.collected) {
            // 모델의 위치(x, y, z)에 모델을 렌더링
            drawModel(model.x, model.y, model.z, model.phase);
        }
    }
}


// 텍스트 그리기 함수
void drawText(const char* text, float x, float y, void* font) {
    glRasterPos2f(x, y);    // 텍스트 위치를 설정
    // 문자열의 각 문자를 순차적으로 출력
    for (const char* p = text; *p; p++) {
        glutBitmapCharacter(font, *p);
    }
}


// 모델 수집 메시지 그리기 함수
void drawCollectedMessage() {
    if (!showCollectedMessage) return;

    // 현재 OpenGL 상태 저장
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);

    // 투영 행렬 변경
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);   // 창 너비
    int height = glutGet(GLUT_WINDOW_HEIGHT); // 창 높이
    gluOrtho2D(0, width, 0, height);          // 2D 화면 설정

    // 모델뷰 행렬 변경
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 조명, 텍스처, 깊이 테스트 비활성화
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // 텍스트 색상을 노란색으로 설정
    glColor3f(1.0f, 1.0f, 0.0f);

    // 수집된 개수 메시지 생성
    std::string message = "Collected: " + std::to_string(collectedCount) + 
            " / " + std::to_string(models.size());
    int len = message.length();
    float textWidth = 9.0f * len;
    float textX = (width - textWidth) / 2.0f; // 너비 중앙에 배치
    float textY = height / 2.0f + 20.0f;      // 높이 중앙에 배치

    // 텍스트 그리기
    drawText(message.c_str(), textX, textY, GLUT_BITMAP_HELVETICA_18);

    // 상태 복원
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // OpenGL 상태 복원
    glPopAttrib();
}


// 목표 도착 메시지 그리기 함수
void drawEscapeMessage() {
    if (!showEscapeMessage) return;

    // 현재 OpenGL 상태 저장
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);

    // 투영 행렬 변경
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);   // 창 너비
    int height = glutGet(GLUT_WINDOW_HEIGHT); // 창 높이
    gluOrtho2D(0, width, 0, height);          // 2D 화면 설정

    // 모델뷰 행렬 변경
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 조명, 텍스처, 깊이 테스트 비활성화
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // 텍스트 색상을 파란색으로 설정
    glColor3f(0.0f, 0.0f, 1.0f);

    // 남은 모델 개수 계산
    int remaining = models.size() - collectedCount;
    std::string message = "You need to collect " + std::to_string(remaining)  + " more model(s) before escaping!";
    int len = message.length();
    float textWidth = 9.0f * len;
    float textX = (width - textWidth) / 2.0f; // 너비 중앙에 배치
    float textY = height / 2.0f + 20.0f;      // 높이 중앙에 배치

    // 텍스트 그리기
    drawText(message.c_str(), textX, textY, GLUT_BITMAP_HELVETICA_18);

    // 상태 복원
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // OpenGL 상태 복원
    glPopAttrib();
}


// 키 입력시 호출 함수
void keyDown(unsigned char key, int x, int y) {
    if (gameFinished) return;  // 게임 끝나면 입력 무시
    if (!inputEnabled) return; // 메시지 표시 중 입력 무시
    keys[key] = true;
}

// 키가 입력 없을시 호출 함수
void keyUp(unsigned char key, int x, int y) {
    if (gameFinished) return;   // 게임 끝나면 입력 무시
    if (!inputEnabled) return;  // 메시지 표시 중 입력 무시
    keys[key] = false;
}


// 카메라와 벽의 충돌 여부 확인 함수
bool checkCollision(float testX, float testZ) {
    float cameraRadius = 0.3f;  // 카메라 충돌 반경 

    // 테스트 위치가 미로의 경계를 벗어났는지 확인
    if (testX < 0 || testZ < 0 || testX > mazeWidth || testZ > mazeHeight) {
        return true;    // 경계 밖으로 나갔다면 충돌로 간주
    }

    // 미로의 모든 셀을 순회하며 벽과의 충돌을 검사
    for (int z = 0; z < mazeHeight; ++z) {      // z축 방향 순회
        for (int x = 0; x < mazeWidth; ++x) {   // x축 방향 순회
            if (maze[z][x] == 1) {      // 해당 셀이 벽인지 확인
                float wallMinX = x - 0.5f;
                float wallMaxX = x + 0.5f;
                float wallMinZ = z - 0.5f;
                float wallMaxZ = z + 0.5f;

                // 테스트 위치와 벽의 경계가 겹치는지 확인
                if (testX + cameraRadius > wallMinX && testX - cameraRadius < wallMaxX &&
                    testZ + cameraRadius > wallMinZ && testZ - cameraRadius < wallMaxZ) {
                    return true; // 충돌이 발생한 경우 true 반환
                }
            }
        }
    }
    // 목표 지점(maze[z][x] == 3)의 오른쪽 벽과의 충돌 검사
    for (int z = 0; z < mazeHeight; ++z) {
        for (int x = 0; x < mazeWidth; ++x) {
            if (maze[z][x] == 3) { // 목표 지점인 경우
                // 목표 지점의 오른쪽 벽 위치 (x + 0.5f)
                float wallX = x + 0.5f;
                float wallMinX = wallX - cameraRadius;
                float wallMaxX = wallX + cameraRadius;
                float wallMinZ = z - 0.5f;
                float wallMaxZ = z + 0.5f;

                // 테스트 위치가 오른쪽 벽과 충돌하는지 확인
                if (testX + cameraRadius > wallMinX && testX - cameraRadius < wallMaxX &&
                    testZ + cameraRadius > wallMinZ && testZ - cameraRadius < wallMaxZ) {
                    return true; // 충돌이 발생한 경우 true 반환
                }
            }
        }
    }
    // 모든 벽과의 충돌이 없으면 false 반환
    return false;
}


// 모델과의 충돌을 검사하는 함수
bool checkModelCollision(float camX, float camZ, float modelX, float modelZ, float collisionDistance) {
    // 두 점 사이의 거리 계산
    float dx = camX - modelX;
    float dz = camZ - modelZ;
    float distance = sqrt(dx * dx + dz * dz);
    return distance < collisionDistance;    // 두 점 사이의 거리가 충돌 거리보다 작으면 충돌 발생
}


// 모델 수집을 체크하는 함수
void checkModelCollection() {
    for (auto& model : models) {
        if (!model.collected && checkModelCollision(cameraX, cameraZ, model.x, model.z)) {
            model.collected = true;
            collectedCount++;

            printf("Collected model! Total: %d\n", collectedCount);

            playCollectSound();     // 모델 먹는 효과음 재생

            // 수집 메시지 표시
            showCollectedMessage = true;
            collectedMessageTimer = 120;
        }
    }
}


// 카메라의 위치/ 방향 업데이트 함수
void updateCamera() {
    if (gameFinished) return;    // 게임이 끝나면 카메라 업데이트 중지

    // 현재 카메라 방향 계산
    float dirX = cos(cameraYaw * M_PI / 180.0f); // 카메라의 x축 방향
    float dirZ = sin(cameraYaw * M_PI / 180.0f); // 카메라의 z축 방향

    // 새로운 카메라 좌표를 저장할 변수 
    float newX = cameraX;
    float newZ = cameraZ;

    // 이동 방향을 저장할 변수
    float moveX = 0.0f;
    float moveZ = 0.0f;

    // 키에 따라 카메라 이동 방향 설정
    if (keys['w']) { // 전진
        moveX += dirX * moveSpeed;
        moveZ += dirZ * moveSpeed;
    }
    if (keys['s']) { // 후진
        moveX -= dirX * moveSpeed;
        moveZ -= dirZ * moveSpeed;
    }
    if (keys['a']) { // 왼쪽 이동
        moveX += dirZ * moveSpeed;
        moveZ -= dirX * moveSpeed;
    }
    if (keys['d']) { // 오른쪽 이동
        moveX -= dirZ * moveSpeed;
        moveZ += dirX * moveSpeed;
    }
    if (keys['q']) { // 좌측 회전
        cameraYaw -= turnSpeed;
    }
    if (keys['e']) { // 우측 회전
        cameraYaw += turnSpeed;
    }

    // 이동 방향이 설정된 경우 충돌을 검사하며 이동
    if (moveX != 0.0f || moveZ != 0.0f) {
        float attemptX = cameraX + moveX; // 이동 시도 x좌표
        float attemptZ = cameraZ + moveZ; // 이동 시도 z좌표

        // 충돌이 없으면 이동 적용
        if (!checkCollision(attemptX, attemptZ)) {
            newX = attemptX;
            newZ = attemptZ;
        }
        // x축 또는 z축 방향으로만 이동 가능하게 하기
        else {
            if (!checkCollision(cameraX + moveX, cameraZ)) {
                newX = cameraX + moveX;
                newZ = cameraZ;
            }
            else if (!checkCollision(cameraX, cameraZ + moveZ)) {
                newX = cameraX;
                newZ = cameraZ + moveZ;
            }
        }
    }

    // 업데이트된 카메라 위치 적용
    cameraX = newX;
    cameraZ = newZ;

    // 현재 카메라 위치를 기반으로 미로 셀 좌표 계산
    int cellX = (int)floor(cameraX + 0.5f);
    int cellZ = (int)floor(cameraZ + 0.5f);

    // 카메라가 미로 내부에 있는 경우 추가 처리
    if (cellZ >= 0 && cellZ < mazeHeight && cellX >= 0 && cellX < mazeWidth) {
        if (maze[cellZ][cellX] == 2) {  // 셀이 용암('2)일 경우
            playWarningSound();   // '2' 밟으면 경고 사운드 재생
            inPauseMode = true;     // 일시정지 모드 활성화
            pauseTimer = 90;        // 일시정지 시간 설정
            // "이런!" 메시지 설정
            showOopsMessage = true;
            oopsMessageTimer = 90;   // 90 프레임 동안 표시 
        }
        else if (maze[cellZ][cellX] == 3 && !gameFinished) { // 셀이 목표('3')일 경우
            if (collectedCount == models.size()) { // 모든 모델을 수집한 경우
                stopBackgroundMusic();
                playSuccessSound();  // 성공 사운드 재생
                escapeTriggered = true;     // 탈출 트리거 활성화
                escapeDelay = 60;   // 탈출 후 지연 시간 설정 
                inputEnabled = false;
            }
            else { // 아직 모델을 모두 수집하지 않은 경우
                showEscapeMessage = true;    // 목표 도착 메시지 표시 설정
                escapeMessageTimer = 120;    // 120 프레임 동안 표시
            }
        }
        else if (maze[cellZ][cellX] == 4) { // 셀이 모델 위치('4')일 경우
            checkModelCollection(); // 모델 수집 체크
        }
    }

    glutPostRedisplay();
}


// 화면 렌더링 함수
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 화면과 깊이 버퍼를 초기화
    glLoadIdentity();

    // 카메라가 바라보는 방향 계산
    float lookX = cameraX + cos(cameraYaw * M_PI / 180.0f); // x축 방향
    float lookZ = cameraZ + sin(cameraYaw * M_PI / 180.0f); // z축 방향

    // 카메라의 뷰 설정
    gluLookAt(cameraX, cameraY, cameraZ, // 카메라 위치
        lookX, cameraY, lookZ,    // 카메라가 바라보는 지점
        0.0f, 1.0f, 0.0f);       // 카메라 상단 방향 (y축)

    // 게임이 종료된 경우
    if (gameFinished) {
        glMatrixMode(GL_PROJECTION); // 투영 행렬 모드로 전환
        glPushMatrix();              // 현재 행렬 저장
        glLoadIdentity();            // 투영 행렬 초기화

        // 2D 화면 설정
        int width = glutGet(GLUT_WINDOW_WIDTH);   // 창 너비
        int height = glutGet(GLUT_WINDOW_HEIGHT); // 창 높이
        gluOrtho2D(0, width, 0, height);          // 화면 좌표계를 2D로 설정

        glMatrixMode(GL_MODELVIEW); // 모델-뷰 행렬 모드로 전환
        glPushMatrix();             // 모델-뷰 행렬 저장
        glLoadIdentity();           // 모델-뷰 행렬 초기화

        // 조명, 텍스처, 깊이 테스트 비활성화
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // 텍스트 색상을 흰색으로 설정
        glColor3f(1.0f, 1.0f, 1.0f);

        const char* finishMessage = "You've escaped from the maze!!"; // 종료 메시지
        int len = (int)strlen(finishMessage);
        float textWidth = 9 * len;
        float textX = (width - textWidth) / 2.0f; // 텍스트를 화면 너비 중앙에 배치
        float textY = height / 2.0f;              // 텍스트를 화면 높이 중앙에 배치

        // 글씨를 크기 조절
        glTranslatef(textX, textY, 0.0f); // 텍스트 위치 설정
        glScalef(1.2f, 1.2f, 1.0f);       // 텍스트 크기 확대

        auto drawBoldText = [&](const char* msg, float ox, float oy) {
            glRasterPos2f(ox, oy);
            for (const char* p = msg; *p; p++) {
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *p);
            }
            };
        drawBoldText(finishMessage, 0.0f, 0.0f);
        drawBoldText(finishMessage, 0.5f, 0.0f);
        drawBoldText(finishMessage, -0.5f, 0.0f);
        drawBoldText(finishMessage, 0.0f, 0.5f);
        drawBoldText(finishMessage, 0.0f, -0.5f);

        // 조명, 텍스처, 깊이 테스트 다시 활성화
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glPopMatrix();  // 모델-뷰 행렬 복원
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();  // 투영 행렬 복원
        glMatrixMode(GL_MODELVIEW);

        glutSwapBuffers();
        return;
    }

    // 게임 진행 중일 경우
    drawGround();       // 땅 그리기
    drawMaze();         // 미로 그리기
    drawOuterWalls();   // 외부 벽 그리기
    drawModels();       // 모델 그리기

    // "이런!" 메시지 표시 여부 확인
    if (showOopsMessage && oopsMessageTimer > 0) {
        oopsMessageTimer--;  // 메시지 타이머 감소

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // 깊이 테스트 및 조명 비활성화
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // 텍스트 색상 설정 (빨간색)
        glColor3f(1.0f, 0.0f, 0.0f);

        // 화면 중앙에 텍스트 "Oops!" 표시
        drawText("Oops! You fell into the lava!!", 280, 300, GLUT_BITMAP_TIMES_ROMAN_24);

        // 상태 복원
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glPopMatrix();  // 모델-뷰 행렬 복원
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();  // 투영 행렬 복원
        glMatrixMode(GL_MODELVIEW);
    }

    // 메시지 타이머가 끝나면 메시지 숨기기
    if (oopsMessageTimer <= 0 && showOopsMessage) {
        showOopsMessage = false;
    }

    // 목표 도착 메시지 표시 여부 확인
    if (showEscapeMessage && escapeMessageTimer > 0) {
        escapeMessageTimer--; // 메시지 타이머 감소

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        int width = glutGet(GLUT_WINDOW_WIDTH);   // 창 너비
        int height = glutGet(GLUT_WINDOW_HEIGHT); // 창 높이
        gluOrtho2D(0, width, 0, height);          // 2D 화면 설정

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // 조명, 텍스처, 깊이 테스트 비활성화
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // 텍스트 색상 설정
        glColor3f(1.0f, 1.0f, 1.0f);

        // 남은 모델 개수 계산
        int remaining = models.size() - collectedCount;
        std::string message = "You need to collect " + std::to_string(remaining) + " more model(s) before escaping!";
        int len = message.length();
        float textWidth = 9.0f * len;
        float textX = (width - textWidth) / 2.0f; // 중앙에 배치
        float textY = height / 2.0f + 20.0f;      // 중앙 약간 위에 배치

        // 텍스트 그리기
        drawText(message.c_str(), textX, textY, GLUT_BITMAP_HELVETICA_18);

        // 상태 복원
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    // 메시지 타이머가 끝나면 메시지 숨기기
    if (escapeMessageTimer <= 0 && showEscapeMessage) {
        showEscapeMessage = false;
    }

    // 초기 메시지가 표시 중인 경우
    if (messageState < 4) {
        // 메시지가 출력될 때 어두운 효과 적용
        GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // 어두운 조명
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

        glMatrixMode(GL_PROJECTION); // 투영 행렬 모드로 전환
        glPushMatrix();              // 현재 행렬 저장
        glLoadIdentity();            // 투영 행렬 초기화

        // 2D 화면 설정
        int width = glutGet(GLUT_WINDOW_WIDTH);   // 창 너비
        int height = glutGet(GLUT_WINDOW_HEIGHT); // 창 높이
        gluOrtho2D(0, width, 0, height);          // 화면 좌표계를 2D로 설정

        glMatrixMode(GL_MODELVIEW); // 모델-뷰 행렬 모드로 전환
        glPushMatrix();             // 모델-뷰 행렬 저장
        glLoadIdentity();           // 모델-뷰 행렬 초기화

        // 조명, 텍스처, 깊이 테스트 비활성화
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // 텍스트 색상을 흰색으로 설정
        glColor3f(1.0f, 1.0f, 1.0f);

        // 메시지 상태에 따라 다른 텍스트를 표시
        const char* message;
        if (messageState == 0) {
            message = "Watch out for the Lava !!!"; // 첫 번째 메시지
        }
        else if (messageState == 1) {
            message = "Use W,A,S,D to move and Q / E to turn!"; // 두 번째 메시지
        }
        else if (messageState == 2) {
            message = "You've to find 3 models to escape!";// 세 번째 메시지
        }
        else {
            message = "Good Luck!"; // 네 번째 메시지
        }

        // 텍스트의 너비와 위치 계산
        int len = (int)strlen(message);
        float textWidthCalc = 9 * len;
        float textX = (width - textWidthCalc) / 2.0f;
        float textY = height / 2.0f;

        drawText(message, textX, textY, GLUT_BITMAP_TIMES_ROMAN_24); // 메시지 그리기

        // 조명, 텍스처, 깊이 테스트 다시 활성화
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glPopMatrix(); // 모델-뷰 행렬 복원
        glMatrixMode(GL_PROJECTION);
        glPopMatrix(); // 투영 행렬 복원
        glMatrixMode(GL_MODELVIEW);
    }
    else {
        // 메시지 표시가 끝난 후 조명 밝기를 원래대로 복구
        GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // 밝은 조명
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    }

    // 모델 수집 메시지 표시
    drawCollectedMessage();

    // 목표 도착 메시지 표시
    drawEscapeMessage();

    glutSwapBuffers();
}


// 타이머 함수
void timer(int value) {
    // 게임이 끝난 상태라면 카메라를 업데이트하지 않음
    if (gameFinished) {
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // 탈출 트리거가 활성화되었지만 게임이 끝나지 않은 경우
    if (escapeTriggered && !gameFinished) {
        escapeDelay--; // 탈출 카운트다운 감소
        if (escapeDelay <= 0) { // 카운트다운이 끝나면
            gameFinished = true; // 게임 종료 상태로 변경
        }
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // 초기 메시지가 표시 중인 경우
    if (messageState < 4) {
        startDelay--;
        if (startDelay == 0) { // 대기 시간이 끝나면
            messageState++;    // 다음 메시지 상태로 전환
            if (messageState < 4) {
                startDelay = 100; // 다음 메시지를 위한 대기 시간 (100프레임)
            }
            else {
                inputEnabled = true; // 메시지가 모두 끝나면 이동키 입력 활성화
            }
        }
    }
    // 메시지가 끝났고 일시정지 상태가 아닌 경우
    else {
        if (!inPauseMode) {
            updateCamera(); // 카메라 위치 업데이트
        }
        // 일시정지 상태인 경우
        else {
            pauseTimer--;
            if (pauseTimer <= 0) {    // 일시정지 타이머가 끝나면
                inPauseMode = false;  // 일시정지 상태 해제
                cameraX = startX;     // 카메라 위치를 시작 위치로 리셋
                cameraY = startY;
                cameraZ = startZ;
                cameraYaw = startYaw; // 카메라 각도 리셋
            }
        }
    }

    // 수집 메시지 타이머 업데이트
    if (showCollectedMessage) {
        collectedMessageTimer--;
        if (collectedMessageTimer <= 0) {
            showCollectedMessage = false;
        }
    }

    // 목표 도착 메시지 타이머 업데이트
    if (showEscapeMessage) {
        escapeMessageTimer--;
        if (escapeMessageTimer <= 0) {
            showEscapeMessage = false;
        }
    }

    glutPostRedisplay(); // 화면 갱신 요청
    glutTimerFunc(16, timer, 0);
}


// 화면 비율 조정 함수
void reshape(int w, int h) {
    glViewport(0, 0, w, h);      // 새로운 창 크기에 맞춰 Viewport 설정
    glMatrixMode(GL_PROJECTION); // 투영 행렬 모드 활성화
    glLoadIdentity();      // 투영 행렬 초기화
    gluPerspective(45.0, (double)w / (double)h, 0.2, 100.0);
    glMatrixMode(GL_MODELVIEW);
}


// 메인 함수
int main(int argc, char** argv) {
    glutInit(&argc, argv);  // GLUT 라이브러리 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);       // 창 크기 설정
    glutCreateWindow("3D Maze - First-Person View");    // 창 생성

    init();
    loadModelFromDAT("model.dat");   // DAT 파일에서 3D 모델 데이터를 로드
    initializeModels();              // 모델 초기화

    // 콜백 함수 등록
    glutDisplayFunc(display); // 화면 렌더링 함수 지정
    glutReshapeFunc(reshape); // 창 크기 변경 시 호출 함수 지정
    glutKeyboardFunc(keyDown); // 키 입력시 호출 함수 지정
    glutKeyboardUpFunc(keyUp); // 키 입력 없을 시 호출 함수 지정
    glutTimerFunc(0, timer, 0); // 타이머 함수 등록 


    glutMainLoop();
    return 0; // 프로그램 종료
}
