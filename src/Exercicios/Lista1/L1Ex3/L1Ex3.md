# Lista 1 - Exercício 3

## Aluno
- Gustavo Rodrigues Lima Haag  

---

## Questão e Resposta

Explique o que é VBO, VAO e EBO, e como se relacionam.

> - **VBO:** _Vertex Buffer Object_, é o buffer de GPU que armazena os vértices. Ele guarda os dados brutos, permitindo que envie para a GPU apenas uma vez (ou quando necessário) ao invés de reenviar a cada "frame". Também permite que a GPU acesse os dados de forma otimizada.  
> - **VAO:** _Vertex Array Object_, ele guarda o estado de como ler esses VBOs: Quais atributos possuem, qual layout, qual VBO está sendo usado para cada atributo, etc. É como um manual de instruções que explica ao OpenGL que para o conjunto de vértices "faça de X maneira, com Y cor...", evitando ter que definir todos os ponteiros de atributo a cada vez que desenha.  
> - **EBO:** _Element Buffer Object_, armazena índices que explicam ao OpenGL qual vértice usar para cada vértice de cada primitiva, ao invés de duplicar vértices se eles fossem usados em mais de uma primitiva.  
> De maneira bem alto nível: Gera o VBO e carrega nele os dados dos vértices -> Gera um VAO e faz um _bind_ ao VAO -> O VAO guarda o vínculo entre os atributos -> Se usar EBO, ele associa com o VAO enquanto ativo, então fica gravado no VAO que as primitivas devem usar os índices do EBO -> Quando desenhar, o VAO é "ativado".