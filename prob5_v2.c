#include <stdio.h>

int findDuplicate(int arr[], int n) {
    int expectedSum = (n - 1) * n / 2; // Suma numerelor 1, 2, ..., n-1
    int actualSum = 0;

    for (int i = 0; i < n; i++) {
        actualSum += arr[i];
    }

    return actualSum - expectedSum; // Diferența este numărul duplicat
}

int main() {
    int arr[] = {1, 2, 3, 4, 2}; 
    int n = sizeof(arr) / sizeof(arr[0]);

    int duplicate = findDuplicate(arr, n);
    printf("Numărul duplicat este: %d\n", duplicate);

    return 0;
}
