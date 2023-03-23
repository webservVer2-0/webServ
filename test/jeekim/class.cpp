#include <iostream>
using namespace std;

class Circle
{
	size_t radius;
	public :
		size_t	GetRad()
		{
			return radius;
		}
		void	SetRad(size_t new_rad)
		{
			radius = new_rad;
		}
};

struct square
{
	void* data;
};

int main() {
	struct square* sq = new(square);
	sq->data = new (Circle);
	Circle* donut = static_cast<Circle*>(sq->data);
	// donut->GetRad();
	donut->SetRad(1);
	// sq->data = static_cast<void*>(donut);
	printf("<3\n");

	return 0;
}
// #include <iostream>
// using namespace std;
// int a = 10;
// void show() {
// 	cout << "쇼" << endl;

// }

// class Circle {
// public:
// 	int radius;
// 	double getArea();
// };

// double Circle::getArea() {
// 	return 3.14 * radius * radius;
// }

// int main() {

// 	Circle donut;
// 	donut.radius = 1;
// 	printf("<3\n");
// 	double area = donut.getArea();
// 	// cout << "donut 면적은 : " << area << endl;

// 	return 0;
// }