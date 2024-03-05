#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char r, g, b;
} RGBPixel;

// Function to read PPM P6 image
int readPPMImage(const char *filename, RGBPixel **pixels, int *width, int *height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        return -1;
    }

    // Read header
    char header[3];
    if (fscanf(fp, "%2s", header) != 1 || header[0] != 'P' || header[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        fclose(fp);
        return -1;
    }

    // Skip comments
    char ch;
    while ((ch = fgetc(fp)) == '#') {
        while (fgetc(fp) != '\n');
    }
    ungetc(ch, fp);

    // Read image size
    if (fscanf(fp, "%d %d", width, height) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        fclose(fp);
        return -1;
    }

    int maxval;
    if (fscanf(fp, "%d", &maxval) != 1) {
        fprintf(stderr, "Invalid max value\n");
        fclose(fp);
        return -1;
    }
    fgetc(fp); // Read the single whitespace after maxval

    // Allocate memory for pixels
    *pixels = (RGBPixel*)malloc(*width * *height * sizeof(RGBPixel));
    if (!*pixels) {
        fprintf(stderr, "Unable to allocate memory\n");
        fclose(fp);
        return -1;
    }

    // Read pixel data
    if (fread(*pixels, sizeof(RGBPixel), *width * *height, fp) != *width * *height) {
        fprintf(stderr, "Error loading image pixels\n");
        free(*pixels);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

// Function to calculate the mean color values of an image
void calculateMeanColor(RGBPixel *pixels, int width, int height, double *meanRed, double *meanGreen, double *meanBlue) {
    long long totalRed = 0, totalGreen = 0, totalBlue = 0;
    int numPixels = width * height;

    for (int i = 0; i < numPixels; i++) {
        totalRed += pixels[i].r;
        totalGreen += pixels[i].g;
        totalBlue += pixels[i].b;
    }

    *meanRed = (double)totalRed / numPixels;
    *meanGreen = (double)totalGreen / numPixels;
    *meanBlue = (double)totalBlue / numPixels;
}

int main() {
    const char *filename = "image-June-08-2023T17-14-07.ppm"; // Replace with your PPM file path
    RGBPixel *pixels = NULL;
    int width, height;
    double meanRed, meanGreen, meanBlue;

    if (readPPMImage(filename, &pixels, &width, &height) != 0) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }

    calculateMeanColor(pixels, width, height, &meanRed, &meanGreen, &meanBlue);
    printf("Mean colors - Red: %.2f, Green: %.2f, Blue: %.2f\n", meanRed, meanGreen, meanBlue);

    free(pixels);
    return 0;
}

