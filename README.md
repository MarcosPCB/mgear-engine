# mGear-1 Suite

> ⚠️ **AVISO:** Este projeto foi descontinuado e pode conter bugs conhecidos. Use por sua conta e risco.

## Visão Geral

O mGear-1 é um conjunto de ferramentas e engines para desenvolvimento de jogos 2D, focado em plataformas Windows, utilizando C/C++ e diversas bibliotecas de terceiros como SDL2, Nuklear, FMOD, GLee, SOIL, stb, entre outras. O projeto inclui um motor de jogo, ferramentas de criação e visualização de sprites, mapas, áudio, além de utilitários para manipulação de arquivos e assets.

Este motor gráfico 2D foi desenvolvido como projeto de estudo, visando ser uma ferramenta poderosa e rápida, totalmente pensada em performance acima de tudo. O mGear utiliza OpenGL 3.3 e técnicas avançadas de luz e sombra para mitigar o uso intenso da GPU, proporcionando alta eficiência mesmo em máquinas modestas.

Além disso, o mGear possui um sistema de script próprio (em desenvolvimento), compilável para bytecode, permitindo automação e customização avançada do comportamento do jogo.

## Estrutura do Projeto

- **mGear-1/**: Engine principal do jogo, com código-fonte do motor, exemplos de assets, shaders, física, UI, etc.
- **mEngineer/**: Ferramenta exclusiva para criação de mapas do jogo.
- **mggcreator/**: Ferramenta para criação de arquivos gráficos MGG (Master Gear Graphics).
- **mgvcreator/**: Ferramenta para criação de vídeos no formato MGV (sequência de JPGs + áudio).
- **mggviewer/**: Visualizador de arquivos MGG.
- **mSprite/**: Ferramenta para animar, criar estados de IA (AI states) e configurar sprites no jogo.
- **mAudio/**: Ferramenta para manipulação e teste de áudio.
- **mTex/**: Ferramenta para manipulação de texturas.
- **mSDK/**: HUB do projeto, conectando as ferramentas da engine com ferramentas do sistema (Visual Studio, Photoshop, After Effects, Illustrator, etc).
- **APIs/**: Bibliotecas de terceiros utilizadas (SDL2, Nuklear, FMOD, stb, SOIL, etc).
- **Test Content/**: Conteúdo de teste, exemplos de arquivos de assets, sprites, músicas, mapas e configurações.

## Principais Funcionalidades

- Engine 2D customizada com suporte a OpenGL 3.3, SDL2, áudio via FMOD, física básica, UI via Nuklear.
- Técnicas de luz e sombra otimizadas para performance, minimizando o uso da GPU.
- Sistema de setores: o motor utiliza um esquema onde o desenvolvedor define retas (segmentos) que determinam onde a física do jogo acontece e quais sprites devem ser renderizados em tela, otimizando o processamento e a lógica de colisão.
- Ferramentas de criação e edição de sprites (com animação, IA e configuração), mapas, texturas, arquivos gráficos e vídeos proprietários.
- Suporte a múltiplos formatos de áudio e imagem.
- Sistema de assets customizado (MGG, MGM, MGV, etc).
- Sistema de script próprio (em desenvolvimento), compilável para bytecode.
- Ferramentas auxiliares para integração e automação de assets.
- HUB de integração (mSDK) com ferramentas externas de criação.

## Como Compilar

1. **Pré-requisitos:**
   - Windows 7 ou superior
   - Visual Studio 2013 ou superior (projetos `.sln` e `.vcxproj`)
   - SDK do Windows, compilador C/C++
   - As bibliotecas de terceiros já estão inclusas em `APIs/`

2. **Compilação:**
   - Abra o arquivo `mGear-1.sln` no Visual Studio.
   - Selecione o projeto desejado (ex: `mGear-1`, `mEngineer`, `mggcreator`, etc).
   - Escolha a configuração (Debug/Release, x86/x64).
   - Compile (Build Solution).
   - Os binários serão gerados nas pastas `bin/` ou `Release/Debug` de cada projeto.

## Como Usar

- **Engine (mGear-1):**
  - Execute o binário gerado para rodar o jogo ou demo.
  - Os assets devem estar nas pastas `data/`, `sprites/`, `font/` e arquivos de configuração na raiz do projeto.

- **Ferramentas:**
  - Cada ferramenta (ex: `mggcreator`, `mggviewer`, `mEngineer`) pode ser executada individualmente para criar, editar ou visualizar assets.
  - Consulte a linha de comando de cada ferramenta para opções (exemplo de uso do `mggcreator`):
    ```
    mggcreator -o "output.mgg" -p "pasta/frames" -i "instrucoes.texprj"
    ```

## Dependências

- SDL2
- SDL2_ttf
- FMOD
- Nuklear
- GLee
- SOIL
- stb
- LibTomCrypt
- curl

Todas as dependências estão inclusas na pasta `APIs/`.

## Créditos

- Marcos (autor principal)
- Bibliotecas de terceiros conforme licenças em `APIs/`

## Licença

Consulte as licenças individuais das bibliotecas em `APIs/`. O código do mGear-1 é de uso privado/educacional, salvo indicação em contrário.

---

**Observação:**
Este projeto é avançado e voltado para desenvolvedores com experiência em C/C++ e engines de jogos. Para dúvidas, consulte os arquivos fonte e exemplos de uso nas ferramentas.

---

# mGear-1 Suite (English)

> ⚠️ **WARNING:** This project has been discontinued and may contain known bugs. Use at your own risk.

## Overview

mGear-1 is a suite of tools and engines for 2D game development, focused on Windows platforms, using C/C++ and several third-party libraries such as SDL2, Nuklear, FMOD, GLee, SOIL, stb, among others. The project includes a game engine, tools for creating and visualizing sprites, maps, audio, and utilities for handling files and assets.

This 2D graphics engine was developed as a study project, aiming to be a powerful and fast tool, with performance as the main priority. mGear uses OpenGL 3.3 and advanced lighting and shadow techniques to mitigate heavy GPU usage, providing high efficiency even on modest machines.

Additionally, mGear features its own scripting system (in development), compilable to bytecode, allowing advanced automation and customization of game behavior.

## Project Structure

- **mGear-1/**: Main game engine, with engine source code, asset examples, shaders, physics, UI, etc.
- **mEngineer/**: Tool exclusively for creating game maps.
- **mggcreator/**: Tool for creating MGG (Master Gear Graphics) graphic files.
- **mgvcreator/**: Tool for creating videos in MGV format (sequence of JPGs + audio).
- **mggviewer/**: Viewer for MGG files.
- **mSprite/**: Tool for animating, creating AI states, and configuring sprites in the game.
- **mAudio/**: Tool for handling and testing audio.
- **mTex/**: Tool for handling textures.
- **mSDK/**: Project HUB, connecting engine tools with system tools (Visual Studio, Photoshop, After Effects, Illustrator, etc).
- **APIs/**: Third-party libraries used (SDL2, Nuklear, FMOD, stb, SOIL, etc).
- **Test Content/**: Test content, sample asset files, sprites, music, maps, and configurations.

## Main Features

- Custom 2D engine with support for OpenGL 3.3, SDL2, audio via FMOD, basic physics, UI via Nuklear.
- Optimized lighting and shadow techniques for performance, minimizing GPU usage.
- Sector system: the engine uses a scheme where the developer defines lines (segments) that determine where game physics occurs and which sprites should be rendered on screen, optimizing processing and collision logic.
- Tools for creating and editing sprites (with animation, AI, and configuration), maps, textures, proprietary graphic and video files.
- Support for multiple audio and image formats.
- Custom asset system (MGG, MGM, MGV, etc).
- Own scripting system (in development), compilable to bytecode.
- Auxiliary tools for asset integration and automation.
- Integration HUB (mSDK) with external creation tools.

## How to Build

1. **Prerequisites:**
   - Windows 7 or higher
   - Visual Studio 2013 or higher (`.sln` and `.vcxproj` projects)
   - Windows SDK, C/C++ compiler
   - Third-party libraries are already included in `APIs/`

2. **Build:**
   - Open the `mGear-1.sln` file in Visual Studio.
   - Select the desired project (e.g., `mGear-1`, `mEngineer`, `mggcreator`, etc).
   - Choose the configuration (Debug/Release, x86/x64).
   - Build Solution.
   - Binaries will be generated in the `bin/` or `Release/Debug` folders of each project.

## How to Use

- **Engine (mGear-1):**
  - Run the generated binary to start the game or demo.
  - Assets must be in the `data/`, `sprites/`, `font/` folders and configuration files in the project root.

- **Tools:**
  - Each tool (e.g., `mggcreator`, `mggviewer`, `mEngineer`) can be run individually to create, edit, or view assets.
  - Check each tool's command line for options (example usage for `mggcreator`):
    ```
    mggcreator -o "output.mgg" -p "folder/frames" -i "instructions.texprj"
    ```

## Dependencies

- SDL2
- SDL2_ttf
- FMOD
- Nuklear
- GLee
- SOIL
- stb
- LibTomCrypt
- curl

All dependencies are included in the `APIs/` folder.

## Credits

- Marcos (main author)
- Third-party libraries as per licenses in `APIs/`

## License

See the individual licenses of the libraries in `APIs/`. The mGear-1 code is for private/educational use unless otherwise stated.

---

**Note:**
This project is advanced and aimed at developers with experience in C/C++ and game engines. For questions, check the source files and usage examples in the tools.
