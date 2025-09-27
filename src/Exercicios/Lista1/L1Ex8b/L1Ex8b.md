# Lista 1 - Exercício 8b

## Aluno
- Gustavo Rodrigues Lima Haag  

---

## Questão e Resposta

Como estes atributos seriam identificados no _vertex shader_?

> Poderiam ser identificados dentro da função `setupGeometry()`:  
> - 3 primeiros valores seriam a posição X, Y e Z. Esse corresponderia ao `layout(location = 0) in vec3 position`;  
> - Os 3 seguintes seriam as cores (R, G, B). E esse corresponderia ao `layout(location = 1) in vec3 color`.