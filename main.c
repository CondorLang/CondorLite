#include <Condor.h>
// #include <stdio.h>

int main(){
	// Scan("var a = 10.0;");
	// Scan("var a = 10; var b = 100;");
	// Scan("var b = 10; var c = 100.0; var a = 10 + 10 + 10 + b + c;");
	// Scan("var a = 10; var b = 100; var apple = 100.0; var banana = 1 + 1 + a;");
	// Scan("var test = 10 / \"b\"");
	// Scan("var test = \"b\"");
	// Scan("var a = 10; for (var i = 0; i < 100; i++) {var d = 100;}");
	// Scan("while (1 == 1 || 2 == 2 || 3 == 3 || 4 == 4) {for (var i = 0; i < 100; i++){int d = 200;}}");
	// Scan("var i = 10; for (i = 0; i < 10; i++){i += 2;} if (i % 2 == 0) {i = 0;} while (true) {var a = 10;}");
	// Scan("while (true || 1 == 1) {int a = 10;}");
	// Scan("int i = 0; for (i; i < 10; i++){i += 1;}");
	// Scan("var i = 0; for (i = 0; i < 10; i++){for (i = 0; i < 10; i++){i += 2;}}");
	// Scan("var i = 0; switch (i) {case 0: return false;}");
	// Scan("var i = 0; if (i == 0) {switch (i) {case 0: break;}}");
	// Scan("func add(int a, int b){return a + b;} func minus(int a, int b){return a + b;} func multiply(int a, int b){return a * b;} for (int i = 0; i < 10; i++){if (i > 0){return 1 + 10;}} while (0 == 0 || 1 == 1 && 2 == 2 && 3 == 3){var a = 10;}");
	
	// Scan("func sumAddOne(int a, int b, int c) return a + b + c + 1; sumAddOne(8,10000, 1);");
	// Scan("func add(int a, int b) return a + b; func multiply(int a, int b) return a * b; multiply(add(1, 1), add(2, 2))");
	Scan("func add(int a, int b) return a + b; func multiply(int x, int y) return x * y; add(1, 1); add(2, 2); add(100, 100); multiply(80, 2)");

	// printf("Press enter to continue...\n");
	// getchar();

	// Scan("var a = 10; for (var i = 0; i < 100; i++) {var d = 100;}");

	// printf("Press enter to continue...\n");
	// getchar();
}
