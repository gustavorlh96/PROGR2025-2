# Lista 1 - Exercício 2

## Aluno
- Gustavo Rodrigues Lima Haag  

---

## Questão e Resposta

O que são primitivas gráficas? Como fazemos o armazenamento dos vértices na OpenGL?

> As primitivas gráficas são elementos básicos de geometria que a pipeline monta para desenhar. Exemplos: Pontos (points), linhas (lines, line strip, line loop, etc.), triângulos (triangles, triangle strip, triangle fan) e as combinações desses. O programador fornece os vértices e o OpenGL monta as primitivas que depois são rasterizadas (convertidas em pixels) para serem impressas na tela. Quanto ao armazenamento dos vértices, na OpenGL moderna, os dados de vértices são enviados para a GPU alocando _buffer objects_, e ali são alocados na memória da GPU.