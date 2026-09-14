#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

// 최대 개수
#define MAX_VERTEX 100
#define MAX_TEXTURE 100
#define MAX_FACE 100

// 정점 좌표
struct Vertex
{
    float x;
    float y;
    float z;
};

// 텍스처 좌표
struct Texture
{
    float s;
    float t;
};

// 삼각형
struct Face
{
    int v[3];   // vertex index
    int t[3];   // texture index
};

int main()
{
    Vertex vertex[MAX_VERTEX];
    Texture texture[MAX_TEXTURE];
    Face face[MAX_FACE];

    int vertexCount = 0;
    int textureCount = 0;
    int faceCount = 0;

    int errorCount = 0;

    char fileName[100];
    char line[256];

    printf("Input file name: ");
    scanf("%s", fileName);


    // =========================================
    // 파일 열기
    // =========================================

    FILE* fp = fopen(fileName, "r");

    if (fp == NULL)
    {
        printf("파일을 열 수 없습니다.\n");
        return 0;
    }


    // =========================================
    // 한 줄씩 읽기
    // =========================================

    int lineNumber = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        lineNumber++;


        // 빈 줄 무시
        if (line[0] == '\n')
            continue;


        // 주석 무시
        if (line[0] == '#')
            continue;


        // =====================================
        // v 데이터
        // =====================================

        if (line[0] == 'v' && line[1] == ' ')
        {
            float x, y, z;

            int result = sscanf(
                line,
                "v %f %f %f",
                &x,
                &y,
                &z
            );


            // 값이 3개가 아닐 경우
            if (result != 3)
            {
                printf(
                    "[Error] line %d : 잘못된 vertex\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            // 좌표 범위 체크
            if (x < -1.0f || x > 1.0f ||
                y < -1.0f || y > 1.0f ||
                z < -1.0f || z > 1.0f)
            {
                printf(
                    "[Error] line %d : vertex 좌표 범위 오류\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            // 배열에 저장
            vertex[vertexCount].x = x;
            vertex[vertexCount].y = y;
            vertex[vertexCount].z = z;

            vertexCount++;
        }


        // =====================================
        // vt 데이터
        // =====================================

        else if (line[0] == 'v' &&
            line[1] == 't' &&
            line[2] == ' ')
        {
            float s, t;

            int result = sscanf(
                line,
                "vt %f %f",
                &s,
                &t
            );


            if (result != 2)
            {
                printf(
                    "[Error] line %d : 잘못된 texture\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            // texture 범위
            if (s < 0.0f || s > 1.0f ||
                t < 0.0f || t > 1.0f)
            {
                printf(
                    "[Error] line %d : texture 좌표 범위 오류\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            texture[textureCount].s = s;
            texture[textureCount].t = t;

            textureCount++;
        }


        // =====================================
        // f 데이터
        // =====================================

        else if (line[0] == 'f' && line[1] == ' ')
        {
            // strtok를 사용하므로
            // 원래 문자열을 직접 잘라서 사용
            char* token;

            token = strtok(line, " \t\n");

            // 첫 번째 token은 f
            // 다음부터 좌표 데이터
            int count = 0;

            int tempV[4];
            int tempT[4];


            // texture가 없음을 표시하기 위해 -1
            for (int i = 0; i < 4; i++)
            {
                tempV[i] = -1;
                tempT[i] = -1;
            }


            while (1)
            {
                token = strtok(NULL, " \t\n");

                if (token == NULL)
                    break;


                // 주석 나오면 뒤는 무시
                if (token[0] == '#')
                    break;


                // 4개 이상이면 삼각형 아님
                if (count >= 4)
                    break;


                int vIndex;
                int tIndex;


                // ---------------------------------
                // f 1/1 형태 검사
                // ---------------------------------

                if (strchr(token, '/') != NULL)
                {
                    int result = sscanf(
                        token,
                        "%d/%d",
                        &vIndex,
                        &tIndex
                    );

                    if (result != 2)
                    {
                        printf(
                            "[Error] line %d : 잘못된 face 데이터\n",
                            lineNumber
                        );

                        errorCount++;

                        count = -1;
                        break;
                    }


                    tempV[count] = vIndex;
                    tempT[count] = tIndex;
                }


                // ---------------------------------
                // f 1 형태
                // ---------------------------------

                else
                {
                    int result = sscanf(
                        token,
                        "%d",
                        &vIndex
                    );

                    if (result != 1)
                    {
                        printf(
                            "[Error] line %d : 잘못된 face 데이터\n",
                            lineNumber
                        );

                        errorCount++;

                        count = -1;
                        break;
                    }


                    tempV[count] = vIndex;

                    // texture 없음
                    tempT[count] = -1;
                }


                count++;
            }


            if (count == -1)
                continue;


            // =====================================
            // 삼각형은 꼭짓점이 3개
            // =====================================

            if (count != 3)
            {
                printf(
                    "[Error] line %d : 삼각형 꼭짓점은 3개여야 합니다.\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            // =====================================
            // 같은 정점 index 검사
            // =====================================

            if (tempV[0] == tempV[1] ||
                tempV[0] == tempV[2] ||
                tempV[1] == tempV[2])
            {
                printf(
                    "[Error] line %d : 같은 vertex index가 있습니다.\n",
                    lineNumber
                );

                errorCount++;
                continue;
            }


            // =====================================
            // vertex index 범위 검사
            // =====================================

            int indexError = 0;

            for (int i = 0; i < 3; i++)
            {
                if (tempV[i] < 1 ||
                    tempV[i] > vertexCount)
                {
                    printf(
                        "[Error] line %d : vertex index 범위 오류\n",
                        lineNumber
                    );

                    indexError = 1;
                    break;
                }
            }


            if (indexError == 1)
            {
                errorCount++;
                continue;
            }


            // =====================================
            // texture index 범위 검사
            // =====================================

            for (int i = 0; i < 3; i++)
            {
                // texture가 있는 경우만 검사
                if (tempT[i] != -1)
                {
                    if (tempT[i] < 1 ||
                        tempT[i] > textureCount)
                    {
                        printf(
                            "[Error] line %d : texture index 범위 오류\n",
                            lineNumber
                        );

                        indexError = 1;
                        break;
                    }
                }
            }


            if (indexError == 1)
            {
                errorCount++;
                continue;
            }


            // =====================================
            // 정상적인 face 저장
            // =====================================

            for (int i = 0; i < 3; i++)
            {
                face[faceCount].v[i] = tempV[i];
                face[faceCount].t[i] = tempT[i];
            }

            faceCount++;
        }


        // =====================================
        // 허용되지 않는 데이터
        // =====================================

        else
        {
            printf(
                "[Error] line %d : 허용되지 않는 문자입니다.\n",
                lineNumber
            );

            errorCount++;
        }
    }


    fclose(fp);



    // =========================================
    // Vertex 값 자체의 중복 검사
    // =========================================

    int duplicate = 0;

    for (int i = 0; i < vertexCount; i++)
    {
        for (int j = i + 1; j < vertexCount; j++)
        {
            if (vertex[i].x == vertex[j].x &&
                vertex[i].y == vertex[j].y &&
                vertex[i].z == vertex[j].z)
            {
                printf(
                    "Duplicate Vertex : %d, %d\n",
                    i + 1,
                    j + 1
                );

                duplicate = 1;
            }
        }
    }



    // =========================================
    // 결과 파일 생성
    // =========================================

    FILE* output = fopen("result.txt", "w");

    if (output == NULL)
    {
        printf("result.txt 파일 생성 실패\n");
        return 0;
    }



    // =========================================
    // Face 출력
    // =========================================

    for (int i = 0; i < faceCount; i++)
    {
        fprintf(
            output,
            "Face %d (%d, %d, %d):\n",
            i + 1,
            face[i].v[0],
            face[i].v[1],
            face[i].v[2]
        );


        // -------------------------------------
        // vertex 좌표 출력
        // -------------------------------------

        fprintf(output, "vertex ");

        for (int j = 0; j < 3; j++)
        {
            // 파일 index는 1부터
            // 배열 index는 0부터
            int index = face[i].v[j] - 1;


            fprintf(
                output,
                "(%.1f, %.1f, %.1f) ",
                vertex[index].x,
                vertex[index].y,
                vertex[index].z
            );
        }

        fprintf(output, "\n");



        // -------------------------------------
        // texture 좌표 출력
        // -------------------------------------

        if (face[i].t[0] != -1)
        {
            fprintf(output, "texture ");

            for (int j = 0; j < 3; j++)
            {
                int index = face[i].t[j] - 1;

                fprintf(
                    output,
                    "(%.1f, %.1f) ",
                    texture[index].s,
                    texture[index].t
                );
            }

            fprintf(output, "\n");
        }

        else
        {
            fprintf(
                output,
                "texture : 없음\n"
            );
        }


        fprintf(output, "\n");
    }



    // =========================================
    // 중복 vertex 결과
    // =========================================

    if (duplicate == 0)
    {
        fprintf(
            output,
            "No duplicate vertex value\n"
        );
    }

    else
    {
        fprintf(
            output,
            "Duplicate vertex value\n"
        );
    }



    fclose(output);



    // =========================================
    // 결과
    // =========================================

    printf("\n");

    printf(
        "Vertex Count : %d\n",
        vertexCount
    );

    printf(
        "Texture Count : %d\n",
        textureCount
    );

    printf(
        "Face Count : %d\n",
        faceCount
    );

    printf(
        "Error Count : %d\n",
        errorCount
    );


    printf(
        "\nresult.txt 파일에 결과를 저장했습니다.\n"
    );


    return 0;
}