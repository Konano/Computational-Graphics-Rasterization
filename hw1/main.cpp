#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define MAXSIZE 2000

queue <pair<int, int> > q;
bool fg[MAXSIZE][MAXSIZE];

class Circle {
public:
    Circle(int x, int y, int r);
    void drawEdge(cv::Mat &img, int r, int g, int b);
    void drawInside(cv::Mat &img, int r, int g, int b);
private:
    int X, Y, R;
    vector<pair<int, int> > v;
    void draw8dir(int x, int y);
    int trasparent(int x, int y);
};

void Circle::draw8dir(int x, int y) {
    v.push_back(make_pair(x + X, y + Y));
    v.push_back(make_pair(-x + X, y + Y));
    v.push_back(make_pair(x + X, -y + Y));
    v.push_back(make_pair(-x + X, -y + Y));
    v.push_back(make_pair(y + X, x + Y));
    v.push_back(make_pair(y + X, x + Y));
    v.push_back(make_pair(-y + X, x + Y));
    v.push_back(make_pair(y + X, -x + Y));
    v.push_back(make_pair(-y + X, -x + Y));
}

Circle::Circle(int _x, int _y, int _r) : X(_x), Y(_y), R(_r) {
    int x = 0, y = R; double d = 1.25 - R;
    draw8dir(x, y);
    while (x <= y) {
        if (d < 0)
            d += 2 * x + 3;
        else
            d += 2 * (x - y) + 5, y--;
        x++;
        draw8dir(x, y);
        draw8dir(x, y - 1); draw8dir(x, y + 1);
    }
    sort(v.begin(), v.end());
    v.erase(unique(v.begin(), v.end()), v.end());
}

inline double dis(int x0, int y0, int x1, int y1) {
    return sqrt(pow(x0 - x1, 2) + pow(y0 - y1, 2));
}

int Circle::trasparent(int x, int y) {
    int transparency = 16;
    if (dis(x-1.0/3, y-1.0/3, X, Y) <= R) transparency -= 1;
    if (dis(x+1.0/3, y-1.0/3, X, Y) <= R) transparency -= 1;
    if (dis(x-1.0/3, y+1.0/3, X, Y) <= R) transparency -= 1;
    if (dis(x+1.0/3, y+1.0/3, X, Y) <= R) transparency -= 1;
    if (dis(x-1.0/3, y, X, Y) <= R) transparency -= 2;
    if (dis(x+1.0/3, y, X, Y) <= R) transparency -= 2;
    if (dis(x, y-1.0/3, X, Y) <= R) transparency -= 2;
    if (dis(x, y+1.0/3, X, Y) <= R) transparency -= 2;
    if (dis(x, y, X, Y) <= R) transparency -= 4;
    return transparency;
}

void Circle::drawEdge(cv::Mat &img, int r, int g, int b) {
    int transparency;
    cv::Vec3b imgP;
    for (int i = 0; i < v.size(); i++) {
        transparency = trasparent(v[i].first, v[i].second);
        imgP = img.at<cv::Vec3b>(v[i].first, v[i].second);
        img.at<cv::Vec3b>(v[i].first, v[i].second)[0] = (imgP[0] * transparency + b * (16 - transparency)) / 16;
        img.at<cv::Vec3b>(v[i].first, v[i].second)[1] = (imgP[1] * transparency + g * (16 - transparency)) / 16;
        img.at<cv::Vec3b>(v[i].first, v[i].second)[2] = (imgP[2] * transparency + r * (16 - transparency)) / 16;
    }
}

void Circle::drawInside(cv::Mat &img, int r, int g, int b) {
    drawEdge(img, r, g, b);
    memset(fg, 0, sizeof(fg));
    for (int i = 0; i < v.size(); i++)
        fg[v[i].first][v[i].second] = 1;
    q.push(make_pair(X, Y)); fg[X][Y] = 1;
    while (!q.empty()) {
        pair<int, int> a = q.front(); q.pop();
        img.at<cv::Vec3b>(a.first, a.second)[0] = b;
        img.at<cv::Vec3b>(a.first, a.second)[1] = g;
        img.at<cv::Vec3b>(a.first, a.second)[2] = r;
        if (!fg[a.first - 1][a.second]) q.push(make_pair(a.first - 1, a.second)), fg[a.first - 1][a.second] = 1;
        if (!fg[a.first + 1][a.second]) q.push(make_pair(a.first + 1, a.second)), fg[a.first + 1][a.second] = 1;
        if (!fg[a.first][a.second - 1]) q.push(make_pair(a.first, a.second - 1)), fg[a.first][a.second - 1] = 1;
        if (!fg[a.first][a.second + 1]) q.push(make_pair(a.first, a.second + 1)), fg[a.first][a.second + 1] = 1;
    }
}

class Polygon {
public:
    Polygon(int sz);
    void insertPoint(int x, int y);
    void drawLine(int x0, int y0, int x1, int y1);
    void drawEdge(cv::Mat &img, int r, int g, int b);
    void drawInside(cv::Mat &img, int r, int g, int b);
private:
    vector<pair<int, int> > v, e;
    vector<int> e_tra;
    int trasparent(const int x0, const int y0, const int x1, const int y1, int x, int y);
    int size;
};

Polygon::Polygon(int sz) :size(sz) {}

void Polygon::insertPoint(int x, int y) {
    v.push_back(make_pair(x, y));
}

inline double dis(const int x0, const int y0, const int x1, const int y1, double x, double y) {
    double cross = (x1 - x0) * (x - x0) + (y1 - y0) * (y - y0);
    if (cross <= 0) return sqrt((x - x0) * (x - x0) + (y - y0) * (y - y0));
    double d2 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    if (cross >= d2) return sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
    double r = cross / d2;
    double px = x0 + r * (x1-x0);
    double py = y0 + r * (y1-y0);
    return sqrt((x - px) * (x - px) + (py - y) * (py - y));
}

int Polygon::trasparent(const int x0, const int y0, const int x1, const int y1, int x, int y) {
    int transparency = 16;
    if (dis(x0, y0, x1, y1, x-1.0/3, y-1.0/3) <= 0.5) transparency -= 1;
    if (dis(x0, y0, x1, y1, x+1.0/3, y-1.0/3) <= 0.5) transparency -= 1;
    if (dis(x0, y0, x1, y1, x-1.0/3, y+1.0/3) <= 0.5) transparency -= 1;
    if (dis(x0, y0, x1, y1, x+1.0/3, y+1.0/3) <= 0.5) transparency -= 1;
    if (dis(x0, y0, x1, y1, x-1.0/3, y) <= 0.5) transparency -= 2;
    if (dis(x0, y0, x1, y1, x+1.0/3, y) <= 0.5) transparency -= 2;
    if (dis(x0, y0, x1, y1, x, y-1.0/3) <= 0.5) transparency -= 2;
    if (dis(x0, y0, x1, y1, x, y+1.0/3) <= 0.5) transparency -= 2;
    if (dis(x0, y0, x1, y1, x, y) <= 0.5) transparency -= 4;
    return transparency;
}

void Polygon::drawLine(int x0, int y0, int x1, int y1) {
    if (x0 == x1) {
        if (y0 > y1) swap(y0, y1);
        for (int y = y0; y <= y1; y++) {
            e.push_back(make_pair(x0, y));
            e_tra.push_back(0);
        }
        return;
    }
    if (y0 == y1) {
        if (x0 > x1) swap(x0, x1);
        for (int x = x0; x <= x1; x++) {
            e.push_back(make_pair(x, y0));
            e_tra.push_back(0);
        }
        return;
    }
    if (x0 > x1) swap(x0, x1), swap(y0, y1);
    int dx = x1 - x0, dy = y1 - y0, d = -dx, x = x0, y = y0;
    for (int i = 0; i <= dx; i++) {
        e.push_back(make_pair(x, y));
        e_tra.push_back(-trasparent(x0, y0, x1, y1, x, y));

        e.push_back(make_pair(x, y + 1));
        e_tra.push_back(trasparent(x0, y0, x1, y1, x, y + 1));
        e.push_back(make_pair(x, y - 1));
        e_tra.push_back(trasparent(x0, y0, x1, y1, x, y - 1));

        x++, d += 2 * (dy > 0 ? dy : -dy);
        if (d >= 0)
            (dy > 0 ? y++ : y--), d -= 2 * dx;
    }
}

void Polygon::drawEdge(cv::Mat &img, int r, int g, int b) {
    e.clear(); e_tra.clear();
    for (int i = 1; i < v.size(); i++)
        drawLine(v[i - 1].first, v[i - 1].second, v[i].first, v[i].second);
    drawLine(v[v.size() - 1].first, v[v.size() - 1].second, v[0].first, v[0].second);
    cv::Vec3b imgP;
    for (int i = 0; i < e.size(); i++) {
        imgP = img.at<cv::Vec3b>(e[i].first, e[i].second);
        img.at<cv::Vec3b>(e[i].first, e[i].second)[0] = (imgP[0] * abs(e_tra[i]) + b * (16 - abs(e_tra[i]))) / 16;
        img.at<cv::Vec3b>(e[i].first, e[i].second)[1] = (imgP[1] * abs(e_tra[i]) + g * (16 - abs(e_tra[i]))) / 16;
        img.at<cv::Vec3b>(e[i].first, e[i].second)[2] = (imgP[2] * abs(e_tra[i]) + r * (16 - abs(e_tra[i]))) / 16;
    }
}

void Polygon::drawInside(cv::Mat &img, int r, int g, int b) {
    drawEdge(img, r, g, b);
    memset(fg, 0, sizeof(fg));
    for (int i = 0; i < e.size(); i++)
        if (e_tra[i] <= 0)
            fg[e[i].first][e[i].second] = 1;
    int X = 0, Y = 0;
    for (int i = 0; i < v.size(); i++)
        X += v[i].first, Y += v[i].second;
    X /= v.size(), Y /= v.size();
    q.push(make_pair(X, Y)); fg[X][Y] = 1;
    while (!q.empty()) {
        pair<int, int> a = q.front(); q.pop();
        img.at<cv::Vec3b>(a.first, a.second)[0] = b;
        img.at<cv::Vec3b>(a.first, a.second)[1] = g;
        img.at<cv::Vec3b>(a.first, a.second)[2] = r;
        if (!fg[a.first - 1][a.second]) q.push(make_pair(a.first - 1, a.second)), fg[a.first - 1][a.second] = 1;
        if (!fg[a.first + 1][a.second]) q.push(make_pair(a.first + 1, a.second)), fg[a.first + 1][a.second] = 1;
        if (!fg[a.first][a.second - 1]) q.push(make_pair(a.first, a.second - 1)), fg[a.first][a.second - 1] = 1;
        if (!fg[a.first][a.second + 1]) q.push(make_pair(a.first, a.second + 1)), fg[a.first][a.second + 1] = 1;
    }
}

int main(int argc, char** argv) {

    const int size = 1600;
    const int width = size;
    const int height = size;

    int triE = 600;
    double triR = (double)triE / sqrt(3);
    double cirI = (double)triE / 18;

    int redMidX = size / 2 - triR;
    int redMidY = size / 2;
    int greenMidX = size / 2 + triR / 2;
    int greenMidY = size / 2 - triE / 2;
    int blueMidX = size / 2 + triR / 2;
    int blueMidY = size / 2 + triE / 2;

    cv::Mat imgRG(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    Circle redInside(redMidX, redMidY, (int)(cirI * 3));
    Circle redOutside(redMidX, redMidY, (int)(cirI * 8));
    redOutside.drawInside(imgRG, 255, 0, 0);
    redInside.drawInside(imgRG, 0, 0, 0);

    Circle greenInside(greenMidX, greenMidY, (int)(cirI * 3));
    Circle greenOutside(greenMidX, greenMidY, (int)(cirI * 8));
    greenOutside.drawInside(imgRG, 0, 255, 0);
    greenInside.drawInside(imgRG, 0, 0, 0);

    Polygon triBig(3);
    triBig.insertPoint(redMidX, redMidY);
    triBig.insertPoint(greenMidX, greenMidY);
    triBig.insertPoint(blueMidX, blueMidY);
    triBig.drawInside(imgRG, 0, 0, 0);

    cv::Mat imgB(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    Circle blueInside(blueMidX, blueMidY, (int)(cirI * 3));
    Circle blueOutside(blueMidX, blueMidY, (int)(cirI * 8));
    blueOutside.drawInside(imgB, 0, 0, 255);
    blueInside.drawInside(imgB, 0, 0, 0);

    Polygon triSmall(3);
    triSmall.insertPoint(blueMidX, blueMidY);
    triSmall.insertPoint(redMidX, redMidY);
    triSmall.insertPoint(redMidX, redMidY + triE);
    triSmall.drawInside(imgB, 0, 0, 0);

    cv::Mat img(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    for (int x = 0; x < height; x++)
        for (int y = 0; y < width; y++)
            for (int o = 0; o < 3; o++)
                img.at<cv::Vec3b>(x, y)[o] = max(imgRG.at<cv::Vec3b>(x, y)[o], imgB.at<cv::Vec3b>(x, y)[o]);

    // Save the image.
    cv::imwrite("pic2.png", img);

    // Show the image, wait for user keystroke and quit.
    cv::imshow("pic2", img);
    cv::waitKey(0);
    cv::destroyAllWindows();
    system("pause");
    return 0;
}