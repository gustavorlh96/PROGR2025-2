# Lista 1 - Exercício 8a

## Aluno
- Gustavo Rodrigues Lima Haag  

---

## Questão e Resposta

Descreva uma possível configuração dos buffers (VBO, VAO e EBO) para representá-lo.

> A configuração possível dos buffers seria utilizar um único buffer (VBO) para os 3 vértices, intercalando as posições (X, Y, Z) e cores (R, G, B). O VAO guardaria os ponteiros de atributos para posição e cor, com 6 floats. O EBO acredito que seria opcional, pois só teríamos 3 vértices não compartilhados.