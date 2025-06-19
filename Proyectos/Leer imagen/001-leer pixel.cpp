#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm> // Para std::clamp (C++17)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

// Función para ajustar el brillo (0.0 = negro, 1.0 = normal, >1.0 más brillante)
unsigned char adjust_brightness(unsigned char pixel, float brightness) {
    float adjusted = pixel * brightness;
    return static_cast<unsigned char>(std::clamp(adjusted, 0.0f, 255.0f));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <imagen.jpg> [salida.jpg] [brillo]\n";
        std::cerr << "Ejemplos:\n";
        std::cerr << "  " << argv[0] << " entrada.jpg\n";
        std::cerr << "  " << argv[0] << " entrada.jpg salida.jpg 1.5\n";
        std::cerr << "  " << argv[0] << " entrada.jpg salida.jpg 0.8\n";
        return 1;
    }

    // Parámetros configurables
    std::string output_file = "grayscale.jpg";
    float brightness = 1.0f; // 1.0 = brillo normal

    // Procesar argumentos
    if (argc >= 3) output_file = argv[2];
    if (argc >= 4) brightness = std::stof(argv[3]);

    std::cout << "Ajustando brillo con factor: " << brightness << "\n";
    std::cout << "  (0.0 = negro total, 1.0 = normal, 2.0 = doble brillo)\n";

    // Iniciar temporización
    auto start = std::chrono::high_resolution_clock::now();

    // Cargar imagen (forzar 3 canales RGB)
    int width, height, orig_channels;
    unsigned char* img = stbi_load(argv[1], &width, &height, &orig_channels, 3);
    auto load_time = std::chrono::high_resolution_clock::now();
    
    if (!img) {
        std::cerr << "Error cargando la imagen: " << stbi_failure_reason() << "\n";
        return 1;
    }

    // Crear buffer para escala de grises
    std::vector<unsigned char> gray_img(width * height);
    
    // Convertir a escala de grises con ajuste de brillo
    for (int i = 0; i < width * height; ++i) {
        const int offset = i * 3;
        
        // Calcular valor de gris (promedio RGB)
        unsigned char gray_value = static_cast<unsigned char>(
            (img[offset] +     // R
             img[offset + 1] + // G
             img[offset + 2])  // B
            / 3
        );
        
        // Aplicar ajuste de brillo
        gray_img[i] = adjust_brightness(gray_value, brightness);
    }
    auto convert_time = std::chrono::high_resolution_clock::now();

    // Guardar imagen en escala de grises
    int success = stbi_write_jpg(output_file.c_str(), width, height, 1, gray_img.data(), 90);
    auto save_time = std::chrono::high_resolution_clock::now();

    if (!success) {
        std::cerr << "Error guardando la imagen: " << output_file << "\n";
    }

    // Calcular duraciones
    auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_time - start);
    auto convert_duration = std::chrono::duration_cast<std::chrono::milliseconds>(convert_time - load_time);
    auto save_duration = std::chrono::duration_cast<std::chrono::milliseconds>(save_time - convert_time);
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(save_time - start);

    // Mostrar resultados
    std::cout << "\nResultados:\n";
    std::cout << "  Dimensiones: " << width << " x " << height << " px\n";
    std::cout << "  Tiempo carga: " << load_duration.count() << " ms\n";
    std::cout << "  Tiempo conversion: " << convert_duration.count() << " ms\n";
    std::cout << "  Tiempo guardado: " << save_duration.count() << " ms\n";
    std::cout << "  Tiempo total: " << total_duration.count() << " ms\n";
    std::cout << "  Imagen guardada como: " << output_file << "\n";

    // Liberar memoria
    stbi_image_free(img);
    
    return 0;
}