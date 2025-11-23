# Trabalho do Grau B

Trabalho desenvolvido para a disciplina **Processamento Gráfico: Fundamentos**.

## 👥 Aluno

- Gustavo Haag

## 💡 Sobre o programa

> O trabalho desenvolvido para o fim do Grau B consiste em um **aplicativo de "edição" de imagens e vídeo** inspirado em aplicativos de câmeras de smartphones, chamado **VIApp** (Video & Image Application).  
> O aplicativo opera em dois modos principais: **Modo Foto** e **Modo Vídeo**. No Modo Vídeo, o usuário pode visualizar a câmera de vídeo em tempo real (simulado por um arquivo de vídeo) e aplicar diversos filtros e overlays. No Modo Foto, além dos filtros, o usuário pode adicionar stickers personalizados à imagem, posicioná-los com o mouse, e salvar a salvar a composição final como uma foto. O programa implementa mais de 16 filtros diferentes de processamento de imagem, incluindo suavização, realce de bordas, efeitos artísticos e detecção de faces.

## 🚀 Como executar 

> Este programa foi feito em **linguagem C++** e pode ser compilado em uma IDE como o **VS Code** (com compilador instalado). Além disso, foi utilizado como base o projeto [PG2025-2](https://github.com/fellowsheep/PG2025-2), então as mesmas dependências e configurações de ambiente são necessárias aqui.

### 📋 Pré-requisitos

#### Bibliotecas necessárias:
- **OpenGL** - Para renderização gráfica
- **GLFW** - Para gerenciamento de janelas e eventos (baixado automaticamente via CMake)
- **GLM** - Para operações matemáticas 3D (baixado automaticamente via CMake)
- **GLAD** - Para carregar extensões OpenGL
- **OpenCV** - Para processamento de imagens e vídeo (**deve ser instalado manualmente**)
- **Dear ImGui** - Para interface gráfica (baixado automaticamente via CMake)

#### Instalação do OpenCV:

**Windows:**
1. Baixe o OpenCV em [opencv.org](https://opencv.org/releases/)
2. Extraia para uma pasta (ex: `C:\opencv`)
3. Adicione `C:\opencv\build\x64\vc16\bin` às variáveis de ambiente PATH
4. Configure a variável `OpenCV_DIR` apontando para `C:\opencv\build`

**Linux:**
```bash
sudo apt-get install libopencv-dev
```

**macOS:**
```bash
brew install opencv
```

### 🔧 Configuração e Build

> **As configurações de ambiente são necessárias para seguir com a execução!**

1. No diretório raíz do projeto `PG2025-2\`, utilize o terminal para navegar até o diretório `build` com o comando:
   ```bash
   cd .\build\
   ```

2. Configure o projeto com CMake (primeira vez apenas):
   ```bash
   cmake ..
   ```

3. Faça o build do projeto:
   ```bash
   cmake --build . --target TGB20252 --config Release
   ```

4. Execute o programa:
   ```bash
   .\TGB20252.exe
   ```

5. O programa ficará executando em uma janela de 540x960 pixels até ser fechado.

## 🎨 Como usar

### 🎥 Modo Vídeo (padrão)

> Ao iniciar o aplicativo, você estará no Modo Vídeo, onde pode:

- **Aplicar Filtros**: Use o dropdown "Filters" no canto superior esquerdo para selecionar entre 16 filtros diferentes
- **Adicionar Overlays**: Use o dropdown "Overlays" para aplicar sobreposições decorativas
- **Detecção de Faces**: Clique no botão "FACE" para ativar/desativar a visualização da detecção de rostos
- **Resetar**: Clique no botão "RESET" para remover todos os filtros e overlays
- **Webcam**: Clique no botão "CAM ON/OFF" para simular ligar/desligar a câmera
- **Trocar Modo**: Clique no botão "PHOTO" para mudar para o Modo Foto

### 📷 Modo Foto

> No Modo Foto, você pode capturar uma imagem e editá-la:

- **Capturar Foto**: Clique no botão central "CAPTURE" para congelar o frame atual
- **Adicionar Stickers**: Selecione um dos 9 stickers disponíveis (S1-S9) e clique na imagem para posicioná-lo
- **Mover Stickers**: Clique e arraste um sticker já posicionado para movê-lo
- **Aplicar Filtros e Overlays**: Funciona da mesma forma que no Modo Vídeo
- **Salvar Imagem**: Clique novamente em "CAPTURE" para salvar a foto editada (formato PNG). As fotos ficam salvas na raíz do projeto `PG2025-2`
- **Voltar ao Vídeo**: Clique no botão "VIDEO" para retornar ao modo de visualização em tempo real

### ⌨️ Controles de Teclado

- `SPACE` - Reseta todos os filtros, overlays e stickers
- `ESC` - Fecha o aplicativo

## 🔍 Filtros Implementados

O aplicativo implementa **16 filtros** diferentes de processamento de imagem:

### Filtros de Suavização:
1. **Bilateral Filtering** - Suaviza pele e fundo mantendo contornos definidos
2. **Box Blur** - Desfoca uniformemente a imagem
3. **Median Blur** - Reduz ruído preservando bordas
4. **Portrait Blur** - Simula modo retrato com fundo desfocado e rosto nítido (requer detecção de face)

### Filtros de Realce:
5. **Sharpen** - Destaca bordas e realça detalhes finos
6. **Laplacian** - Realça áreas de transição rápida de intensidade
7. **Sobel** - Detecta bordas horizontais e verticais
8. **Canny Edge** - Extrai bordas com alta precisão

### Filtros de Cor e Tonalidade:
9. **B&W (Grayscale)** - Converte para tons de cinza equilibrados
10. **Vintage (Sepia)** - Aplica tonalidade quente inspirada em filme antigo
11. **Negative (Invert)** - Inverte as cores para um efeito experimental
12. **RGB Channels** - Liga ou desliga rapidamente cada canal de cor (R, G, B)

### Filtros de Ajuste:
13. **Bright (Brightness)** - Eleva o brilho geral de maneira suave
14. **Contrast** - Amplifica contraste e profundidade
15. **Emboss** - Cria relevo simulando iluminação lateral

### Filtros Especiais:
16. **VHS** - Simula fita analógica com bleeding de cores, scanlines e ruído

### Seleção de Canais

Todos os filtros podem ser aplicados em:
- **Imagem completa (RGB)**
- **Canal vermelho (R)** apenas
- **Canal verde (G)** apenas  
- **Canal azul (B)** apenas
- **Tons de cinza (Grayscale)**

## 🎭 Stickers Disponíveis

O aplicativo inclui **9 stickers** temáticos com suporte a transparência (canal alfa):

1. **Aperture** - Logo da Aperture Science (Portal)
2. **Cat** - Gatinho fofo (com marca d'agua, não vou pagar por isso)
3. **Chicken Jockey** - Personagem do Minecraft (famoso por situações caóticas no cinema)
4. **Companion Cube** - Cubo de companhia (Portal)
5. **C++ Logo** - Logo da linguagem C++
6. **HL3** - Ainda tenho esperanças do Half-Life 3 existir
7. **Hollow Knight** - Personagem do jogo Hollow Knight
8. **Nyan Cat** - O famoso meme do gato (bolacha?) espacial
9. **OpenCV Logo** - Logo da biblioteca OpenCV

## 🖼️ Overlays Implementados

O aplicativo oferece **3 overlays** decorativos:

1. **HLA Glyph** - Símbolo semelhante ao da capa principal do jogo Half-Life: Alyx
2. **Hipster** - Sobreposição estilo hipster de 2010 (sdds tumblr)
3. **Summer** - Sobreposição temática de verão (tá bem baixa a resolução, desculpa)

## 🧩 Funcionalidades Extras Implementadas

Além dos requisitos mínimos, o projeto implementa as seguintes funcionalidades extras:

✅ **Interface Gráfica com Dear ImGui** - Interface moderna e intuitiva  
✅ **Combinação de Filtros** - Filtros podem ser aplicados em sequência com overlays
✅ **Filtros com Detecção de Face** - Modo Portrait que detecta rostos e aplica desfoque seletivo usando OpenCV Haar Cascades  
✅ **Stickers Interativos** - Sistema completo de posicionamento, movimentação e prévia de stickers

## 🏗️ Arquitetura do Projeto

O projeto está organizado em módulos especializados:

```
TGB20252/
├── tgb20252.cpp          # Arquivo principal com a classe VIApp
├── FilterManager.*       # Gerenciamento de filtros de imagem
├── StickerManager.*      # Gerenciamento de stickers
├── OverlayManager.*      # Gerenciamento de overlays decorativos
├── VideoHandler.*        # Manipulação de vídeo e frames
├── TextureManager.*      # Gerenciamento de texturas OpenGL
├── FaceDetector.*        # Detecção de faces com OpenCV
├── ImageOperations.*     # Operações matemáticas com imagens
├── UIManager.*           # Gerenciamento da interface (não utilizado)
└── Sprite.*              # Estruturas de dados para sprites
```

## 🎓 Tecnologias Utilizadas

- **C++17** - Linguagem de programação
- **OpenGL 4.0** - Renderização gráfica
- **OpenCV 4.x** - Processamento de imagens e detecção de faces
- **Dear ImGui** - Interface gráfica moderna
- **GLFW** - Gerenciamento de janelas e eventos
- **GLM** - Matemática para gráficos 3D
- **CMake** - Sistema de build

## 🎯 Conceitos de Processamento de Imagem Aplicados

O projeto explora diversos conceitos fundamentais:

- **Convolução** - Aplicação de kernels para blur, sharpen e emboss
- **Detecção de Bordas** - Operadores Sobel, Laplacian e Canny
- **Filtragem Espacial** - Box filter, Median filter, Gaussian blur
- **Transformações de Cor** - Conversão RGB↔Gray, matriz Sepia
- **Operações Aritméticas** - Adição ponderada, blending com canal alfa
- **Máscaras e ROI** - Aplicação seletiva de filtros usando máscaras de face
- **Detecção de Objetos** - Haar Cascades para detecção facial
- **Manipulação de Canais** - Separação e recombinação de canais RGB

## 📚 Referências

- [OpenCV Documentation](https://docs.opencv.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [Learn OpenGL](https://learnopengl.com/)
- [Creating faux analogue video imagery with python](https://polprog.net/blog/pyvideo/)
- Material da disciplina Processamento Gráfico: Fundamentos

## 🎮 Isso é tudo!

_"The right code in the wrong place can make all the difference in the world."_ **— Adaptado de Half-Life 2**
