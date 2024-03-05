#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GRAY_LEVELS 256

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

    char header[3];
    if (fscanf(fp, "%2s", header) != 1 || header[0] != 'P' || header[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        fclose(fp);
        return -1;
    }

    char ch;
    while ((ch = fgetc(fp)) == '#') {
        while (fgetc(fp) != '\n');
    }
    ungetc(ch, fp);

    if (fscanf(fp, "%d %d", width, height) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        fclose(fp);
        return -1;
    }

    int maxval;
    if (fscanf(fp, "%d", &maxval) != 1 || maxval != 255) {
        fprintf(stderr, "Invalid max value\n");
        fclose(fp);
        return -1;
    }
    fgetc(fp); // Read the single whitespace after maxval

    *pixels = (RGBPixel*)malloc(*width * *height * sizeof(RGBPixel));
    if (!*pixels) {
        fprintf(stderr, "Unable to allocate memory\n");
        fclose(fp);
        return -1;
    }

    if (fread(*pixels, sizeof(RGBPixel), *width * *height, fp) != *width * *height) {
        fprintf(stderr, "Error loading image pixels\n");
        free(*pixels);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

// Function to convert RGB to grayscale
void rgbToGrayscale(RGBPixel *pixels, unsigned char *grayImage, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        grayImage[i] = (unsigned char)(0.3 * pixels[i].r + 0.59 * pixels[i].g + 0.11 * pixels[i].b);
    }
}

// A function to calculate the GLCM (Gray Level Co-occurrence Matrix)
void calculateGLCM(unsigned char *grayImage, int width, int height, int distance, int angle, double glcm[GRAY_LEVELS][GRAY_LEVELS]) {
    int dx = 0, dy = 0;

    // Setting direction based on angle
    if(angle == 0) { dx = distance; dy = 0; } // Horizontal
    else if(angle == 90) { dx = 0; dy = -distance; } // Vertical
    // Add more directions (angles) if needed

    for(int i = 0; i < GRAY_LEVELS; i++)
        for(int j = 0; j < GRAY_LEVELS; j++)
            glcm[i][j] = 0.0;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int newX = x + dx;
            int newY = y + dy;

            if(newX >= 0 && newX < width && newY >= 0 && newY < height) {
                int currentValue = grayImage[y * width + x];
                int newValue = grayImage[newY * width + newX];
                glcm[currentValue][newValue]++;
            }
        }
    }

    // Normalize the GLCM
    double sum = 0.0;
    for(int i = 0; i < GRAY_LEVELS; i++) {
        for(int j = 0; j < GRAY_LEVELS; j++) {
            sum += glcm[i][j];
        }
    }

    if(sum != 0.0) { // Prevent division by zero
        for(int i = 0; i < GRAY_LEVELS; i++) {
            for(int j = 0; j < GRAY_LEVELS; j++) {
                glcm[i][j] /= sum;
            }
        }
    }
}

// Function to calculate texture features from GLCM
void calculateTextureFeatures(double glcm[GRAY_LEVELS][GRAY_LEVELS], double *contrast, double *correlation, double *energy) {
    *contrast = 0.0;
    *correlation = 0.0;
    *energy = 0.0;
    double mean = 0.0, std = 0.0, sum = 0.0;

    // Calculate mean
    for(int i = 0; i < GRAY_LEVELS; i++) {
        for(int j = 0; j < GRAY_LEVELS; j++) {
            mean += i * glcm[i][j];
        }
    }

    // Calculate standard deviation
    for(int i = 0; i < GRAY_LEVELS; i++) {
        for(int j = 0; j < GRAY_LEVELS; j++) {
            std += glcm[i][j] * (i - mean) * (i - mean);
        }
    }
    std = sqrt(std);

    // Calculate contrast, correlation, and energy
    for(int i = 0; i < GRAY_LEVELS; i++) {
        for(int j = 0; j < GRAY_LEVELS; j++) {
            *contrast += (i - j) * (i - j) * glcm[i][j];
            if(std != 0) { // Prevent division by zero
                *correlation += ((i - mean) * (j - mean) * glcm[i][j]) / (std * std);
            }
            *energy += glcm[i][j] * glcm[i][j];
        }
    }
}

int main() {
    const char *filename = "image-June-08-2023T17-14-07.ppm"; // Replace with your PPM file path
    RGBPixel *pixels = NULL;
    int width, height;

    if (readPPMImage(filename, &pixels, &width, &height) != 0) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }

    unsigned char *grayImage = (unsigned char *)malloc(width * height);
    if (!grayImage) {
        fprintf(stderr, "Unable to allocate memory for grayscale image\n");
        free(pixels);
        return 1;
    }

    rgbToGrayscale(pixels, grayImage, width, height);
    free(pixels); // Free the original image memory

    double glcm[GRAY_LEVELS][GRAY_LEVELS];
    calculateGLCM(grayImage, width, height, 1, 0, glcm); // Example with distance=1 and angle=0
    free(grayImage); // Free the grayscale image memory

    double contrast, correlation, energy;
    calculateTextureFeatures(glcm, &contrast, &correlation, &energy);

    printf("Texture Features:\n");
    printf("Contrast: %f\n", contrast);
    printf("Correlation: %f\n", correlation);
    printf("Energy: %f\n", energy);

    return 0;
}

