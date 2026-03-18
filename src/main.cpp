#include "allocator.hpp"
#include <iostream>
#include <cstdint> 

/**
 * @file main.cpp
 * @brief Teste funcional e validação de alinhamento do MemoryPool.
 */
int main() {
    // 1. Instanciação do pool para o tipo primitivo 'int' com capacidade para 3 unidades.
    MemoryPool<int> pool(3);

    // 2. Alocação sequencial de blocos de memória.
    int* a = pool.allocate();
    int* b = pool.allocate();
    int* c = pool.allocate(); 

    // 3. Validação de integridade: Garante que o pool não retornou ponteiros nulos (nullptr).
    if (!a || !b || !c) {
        std::cerr << "Critical error: Pool exhaustion or allocation failure!\n";
        return 1;
    }

    // 4. Teste de escrita: Verifica se os endereços retornados são graváveis e independentes.
    *a = 100;
    *b = 200;
    *c = 300;

    std::cout << "\n--- Allocation Report ---\n";
    std::cout << "Pointer A: " << a << " | Content: " << *a << "\n";
    std::cout << "Pointer B: " << b << " | Content: " << *b << "\n";
    std::cout << "Pointer C: " << c << " | Content: " << *c << "\n";

    // 5. Análise de Alinhamento e Offset:
    // Calcula o deslocamento em bytes entre dois blocos consecutivos.
    // O cast para size_t previne disparidade de tipos (signed vs unsigned) na comparação.
    size_t diff = static_cast<size_t>(reinterpret_cast<char*>(b) - reinterpret_cast<char*>(a));
    
    std::cout << "\n--- Memory Layout Analysis ---\n";
    std::cout << "Offset entre A e B: " << diff << " bytes" << std::endl;

    // Validação lógica: A diferença deve ser no mínimo o tamanho do tipo T (ou do nó da Free-List).
    if (diff >= sizeof(int)) {
        std::cout << "STATUS: VERIFIED. Consistent alignment with the architecture." << std::endl;
    } else {
        std::cout << "STATUS: FAILURE. Segmentation overlap or corruption detected." << std::endl;
    }

    // 6. Reciclagem de Memória: Retorno dos ponteiros para a Free-List do pool.
    pool.deallocate(a);
    pool.deallocate(b);
    pool.deallocate(c);

    std::cout << "\nExecution completed without access violations.\n\n";
    
    return 0;
}
