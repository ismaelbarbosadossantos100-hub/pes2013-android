# PES 2013 ReXGlue → Android ARM64 — Pacote de continuidade

**Projeto:** port nativo de Pro Evolution Soccer 2013 recompilado de Xbox 360 para Android ARM64.

**Alvo principal:** MediaTek Helio G99, GPU Mali-G57 MC2, 4 GB de RAM, Android 16/API 36.

**Responsável/créditos da interface:** Shisui Bahiano — https://youtube.com/@shisui-bahiano

**Estado deste documento:** 18 de outubro de 2026.

## 1. Escopo e autorização

O trabalho parte da premissa declarada pelo proprietário de que possui os direitos ou autorização necessários para analisar, modificar e usar o `default.xex`, arquivos de dados e recursos do PES 2013. O código-fonte, os dados do jogo e qualquer redistribuição devem continuar sendo tratados de acordo com essa autorização. O pacote de continuidade não inclui a imagem do jogo nem a pasta `game_data`.

Não redistribuir arquivos proprietários do jogo fora do ambiente autorizado. O usuário deve obter os dados do jogo por sua própria fonte autorizada e importá-los pelo launcher.

## 2. Resultado alcançado

- `default.xex` analisado; Media ID registrado: `5B68C6D5`.
- Recompilação estática realizada pelo ReXGlue.
- Aproximadamente 272 partições C++ geradas, totalizando aproximadamente 284 MB de código gerado.
- Build host Linux funcional usado para verificar o código recompilado.
- Ambiente Android preparado com NDK r26c, API 34 e Gradle 8.12.
- APK `arm64-v8a` funcional no sentido de instalar e abrir o launcher.
- Launcher Android com seleção de pasta via Storage Access Framework.
- Importação recursiva dos dados para o armazenamento específico do aplicativo.
- Verificação de `default.xex` e tamanho do arquivo antes do Play.
- `LauncherActivity` e `GameActivity` separadas.
- SDL3 linkado estaticamente em `libmain.so`; o app não procura uma biblioteca separada `libSDL3.so`.
- Logger Java com sessões, exceções e caminho do último log.
- Backend gráfico ReXGlue/Xenos usando Vulkan.
- APK sem PanVK, sem AdrenoTools e sem driver Mali empacotado.

## 3. Estado atual do problema

O launcher funciona e encontra o jogo:

```text
XEX_CHECK path=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data/default.xex exists=true bytes=20791296
```

Os argumentos também chegam ao SDL:

```text
GAME_ARGUMENTS game_data_root=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data native_log=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/logs/pes2013_20261018_031600.log vulkan_loader=system_libvulkan.so
```

Depois disso, a `GameActivity` é destruída aproximadamente 0,5–1 segundo depois. O log Java ainda não mostrou uma mensagem nativa conclusiva. Por isso, o APK mais recente registra também:

```text
SDL_JNI_ENTER
SDL_JNI_RETURN
SDL_JNI_THROWABLE ...
```

O código nativo também contém marcadores `NATIVE_STAGE` e `PES2013_NATIVE`. O APK foi verificado estaticamente e contém essas strings em `libmain.so`.

### Hipóteses prioritárias

1. Falha antes ou durante a chamada JNI para `SDL_main`.
2. Retorno imediato de `SDL_main` porque `SDLWindowedAppContext::Initialize()` ou `ReXApp::OnInitialize()` falha.
3. Falha na criação da superfície SDL/Vulkan.
4. Erro do logger nativo ao abrir o arquivo externo, mascarando a mensagem real.
5. Falha posterior no carregamento do XEX ou no runtime recompilado.
6. Crash nativo sem exceção Java, que exige logcat para diagnóstico definitivo.

A hipótese PanVK foi investigada e não é sustentada pelo APK atual: não há PanVK no pacote, e o ReXGlue abre `libvulkan.so`, o loader Vulkan do sistema Android.

## 4. Vulkan e plataforma

O loader usado pelo ReXGlue é definido em `include/rex/platform/dynlib.h`:

```cpp
#if REX_PLATFORM_ANDROID
inline constexpr const char* kVulkanLoader = "libvulkan.so";
#endif
```

O Android define simultaneamente `REX_PLATFORM_ANDROID=1` e `REX_PLATFORM_LINUX=1`, pois o código usa Linux/POSIX como base. Isso é intencional; os blocos `REX_PLATFORM_ANDROID` têm prioridade quando necessário. Não substituir esse comportamento sem revisar todas as condicionais do SDK.

O APK final contém `librexgpu-xenosd.so`, `librexruntimed.so`, `libmain.so` e `libTracyClientd.so`, mas não contém `libvulkan.so`, PanVK, Panfrost, Mali blob ou AdrenoTools. Assim, o Vulkan é resolvido pelo Android e pelo driver Mali-G57 instalado no aparelho.

## 5. Arquivos principais do projeto

### Projeto PES 2013

- `/home/ubuntu/pes2013_rex/src/main.cpp`
- `/home/ubuntu/pes2013_rex/src/pes2013_app.h`
- `/home/ubuntu/pes2013_rex/pes2013_manifest.toml`
- `/home/ubuntu/pes2013_rex/manual_functions.toml`
- `/home/ubuntu/pes2013_rex/generated/rexglue.cmake`
- `/home/ubuntu/pes2013_rex/generated/default/` — código C++ gerado e `sources.cmake`
- `/home/ubuntu/pes2013_rex/CMakeLists.txt`
- `/home/ubuntu/pes2013_rex/CMakePresets.json`

### Plano e Android

- `/home/ubuntu/pes2013_rex/ANDROID_PORT_PLAN.md`
- `/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/LauncherActivity.java`
- `/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/GameActivity.java`
- `/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/SDLActivity.java`
- `/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/SessionLogger.java`
- `/home/ubuntu/pes2013_rex/android/app/src/main/AndroidManifest.xml`
- `/home/ubuntu/pes2013_rex/android/app/src/main/cpp/CMakeLists.txt`
- `/home/ubuntu/pes2013_rex/android/app/build.gradle`
- `/home/ubuntu/pes2013_rex/android/gradle.properties`

### Runtime ReXGlue modificado/compilado

- `/home/ubuntu/work_rexglue_gcc14/src/`
- `/home/ubuntu/work_rexglue_gcc14/include/`
- `/home/ubuntu/work_rexglue_gcc14/cmake/`
- `/home/ubuntu/work_rexglue_gcc14/src/ui/rex_app.cpp`
- `/home/ubuntu/work_rexglue_gcc14/src/ui/windowed_app_main_sdl.cpp`
- `/home/ubuntu/work_rexglue_gcc14/src/ui/vulkan/vulkan_instance.cpp`
- `/home/ubuntu/work_rexglue_gcc14/src/core/logging.cpp`
- `/home/ubuntu/work_rexglue_gcc14/include/rex/platform.h`
- `/home/ubuntu/work_rexglue_gcc14/include/rex/platform/dynlib.h`

### Skill

- `/home/ubuntu/skills/pes2013-android-port/SKILL.md`

## 6. Requisitos de compilação

- Linux x86_64.
- CMake compatível.
- GCC 14 para o build host quando C++23 for necessário.
- Android SDK.
- Android NDK r26c: `26.2.11394342`.
- Android API 34 configurada para compilação.
- Gradle 8.12.
- ABI: somente `arm64-v8a`.
- ReXGlue SDK apontando para `/home/ubuntu/work_rexglue_gcc14` no ambiente original.

## 7. Build host

O host deve ser compilado antes do Android para validar o codegen e o runtime:

```bash
cd pes2013_rex
cmake --preset default
cmake --build --preset default -j"$(nproc)"
```

Os nomes exatos dos presets devem ser conferidos em `CMakePresets.json`. O host anterior foi compilado usando GCC 14.

## 8. Build Android

No ambiente original:

```bash
cd /home/ubuntu/pes2013_rex/android
/home/ubuntu/.local/gradle/gradle-8.12/bin/gradle --no-daemon assembleDebug
```

APK produzido:

```text
/home/ubuntu/pes2013_rex/android/app/build/outputs/apk/debug/app-debug.apk
```

O último APK diagnóstico compilado tem SHA-256:

```text
f2c8ca89bec5fe473d71d8f7a6c13e428d03aba8fee39d2cd7eead1dc6c99991
```

Como `local.properties` contém caminhos específicos do ambiente, ele deve ser recriado na máquina de continuidade. Não confiar no caminho absoluto original.

## 9. Teste apenas pelo celular

1. Instalar o APK por cima da versão anterior para preservar `game_data`.
2. Abrir o launcher.
3. Pressionar Play uma única vez.
4. Aguardar o fechamento.
5. Abrir o launcher novamente.
6. Usar o botão de mostrar o último log.
7. Enviar o conteúdo inteiro do arquivo.

O caminho normal do log é:

```text
/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/logs/
```

As linhas importantes são `SDL_JNI_ENTER`, `SDL_JNI_RETURN`, `SDL_JNI_THROWABLE`, `NATIVE_STAGE` e `APP_INITIALIZE_RESULT`.

Sem computador, o APK não consegue ler integralmente o logcat do Android por limitações de permissão. O logger próprio cobre Java e agora cobre as etapas de entrada JNI; um crash SIGSEGV ainda pode exigir logcat/ADB em algum momento.

## 10. Controles e resolução planejados

A implementação planejada deve manter separadas:

- resolução interna do jogo;
- resolução da superfície Android;
- escala e aspect ratio;
- letterbox/pillarbox 16:9;
- cache de shaders por GPU, driver, resolução e configuração.

O overlay Xbox 360 deve incluir analógico esquerdo, analógico direito, D-pad, A/B/X/Y, LB/RB, LT/RT, Start, Back e Guide opcional. Deve haver escala, opacidade, posição, zona morta e vibração configuráveis. Os controles não devem escrever diretamente na memória guest; devem passar pelo sistema de input/SDL/ReXGlue.

## 11. Próxima sequência técnica recomendada

1. Testar o último APK e verificar `SDL_JNI_ENTER`.
2. Se houver `SDL_JNI_THROWABLE`, corrigir a exceção Java/JNI.
3. Se houver `SDL_JNI_ENTER` sem retorno, confirmar no logcat ou adicionar uma ponte nativa ainda mais precoce.
4. Se aparecer `NATIVE_STAGE SDL_CONTEXT_INIT_FAILED`, corrigir inicialização de SDL/superfície.
5. Se aparecer `APP_INITIALIZE_RESULT=0`, analisar `SetupEnvironment`, `SetupPresentation`, `ConstructRuntime` e `LaunchModule`.
6. Se Vulkan falhar, registrar `VkResult`, GPU, API version, extensões e nome do dispositivo no arquivo Java/nativo.
7. Se o runtime inicializar e falhar no XEX, analisar imports, memória guest, boundaries e módulos carregados.
8. Só depois de obter tela funcional implementar controles virtuais e resolução personalizada.
9. Testar primeiro em modo conservador: Vulkan do sistema, 1280x720, sem PanVK, sem shaders experimentais.
10. Criar release somente depois que o debug iniciar e permanecer no loop principal.

## 12. Integridade e cautelas

- Não apagar o aplicativo durante os testes se os dados importados forem importantes.
- Não incluir `game_data`, `default.xex` ou recursos proprietários em repositórios públicos.
- Não empacotar driver de outra GPU no APK Mali.
- Não tratar o APK atual como port final: ele é um build de diagnóstico com launcher funcional, mas o runtime ainda encerra durante a inicialização.
- O código gerado é grande; o arquivo `generated/default/` é necessário se não houver acesso ao codegen original.

## 13. Identificação do pacote

Pacote Android: `com.tokyogames.pes2013`

Activity launcher: `org.libsdl.app.LauncherActivity`

Activity do jogo: `org.libsdl.app.GameActivity`

ABI: `arm64-v8a`

GPU foco: Mali-G57 MC2

Backend foco: Vulkan do sistema Android

Crédito exibido: Shisui Bahiano
