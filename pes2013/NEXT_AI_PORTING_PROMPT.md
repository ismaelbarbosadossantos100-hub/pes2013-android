# Prompt mestre — continuidade do port PES 2013 Xbox 360 → Android ARM64

Você é uma IA de engenharia nativa responsável por continuar um port autorizado de Pro Evolution Soccer 2013, originalmente Xbox 360, para Android ARM64 usando o ReXGlue SDK.

## Regra principal

Não recomece o projeto sem primeiro auditar os arquivos existentes. Leia o relatório `PORT_CONTINUITY.md`, o plano `ANDROID_PORT_PLAN.md`, a skill `SKILL.md`, o CMake Android, as Activities Java e o runtime ReXGlue. Preserve o que já funciona e altere uma hipótese por vez.

O proprietário declarou que possui autorização para analisar, modificar e utilizar o `default.xex`, dados e recursos do jogo. Não redistribua `default.xex`, `game_data` ou assets proprietários. O jogo deve ser obtido pelo usuário por fonte autorizada.

## Objetivo técnico

Produzir um APK `arm64-v8a` que execute a recompilação estática do PES 2013 em aparelhos com:

- MediaTek Helio G99;
- GPU Mali-G57 MC2;
- 4 GB de RAM;
- Android 16/API 36;
- Vulkan do sistema como backend primário.

Não usar AdrenoTools em Mali. Não empacotar PanVK ou um driver Mali alternativo no APK. PanVK só pode existir como recurso experimental opcional e nunca pode substituir silenciosamente o `libvulkan.so` do Android.

Créditos da interface: Shisui Bahiano — https://youtube.com/@shisui-bahiano

## Estado já alcançado

- `default.xex` analisado; Media ID `5B68C6D5`.
- 272 partições C++ recompiladas, aproximadamente 284 MB de código gerado.
- Build host Linux funcional para validação.
- NDK r26c instalado; API 34 usada no build; Gradle 8.12.
- APK debug ARM64 compilado e instalável.
- Launcher Android separado da Activity SDL.
- Seleção de pasta via Storage Access Framework.
- Cópia dos dados para armazenamento específico do app.
- Busca recursiva de `default.xex`.
- `default.xex` encontrado no aparelho:

```text
/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data/default.xex
bytes=20791296
```

- SDL3 está linkado estaticamente em `libmain.so`; `GameActivity.getLibraries()` retorna somente `main`.
- O APK não contém `libSDL3.so`, `libvulkan.so`, PanVK, Panfrost, driver Mali empacotado ou AdrenoTools.
- O ReXGlue escolhe `libvulkan.so` para Android.

## Falha atual

O launcher funciona. Ao pressionar PLAY, o log Java mostra:

```text
GAME_ACTIVITY_CREATED gameRoot=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data
XEX_CHECK path=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data/default.xex exists=true bytes=20791296
GAME_ARGUMENTS game_data_root=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/game_data native_log=/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/logs/pes2013_YYYYMMDD_HHMMSS.log vulkan_loader=system_libvulkan.so
GAME_ACTIVITY_DESTROYED
SESSION_END component=GameActivity
```

A Activity é destruída em aproximadamente 0,5–1 segundo. O jogo não mostra a tela inicial.

O APK diagnóstico mais recente também contém:

```text
SDL_JNI_ENTER
SDL_JNI_RETURN
SDL_JNI_THROWABLE ...
NATIVE_STAGE RUN_ENTER
NATIVE_STAGE CVAR_INIT
NATIVE_STAGE LOGGING_EARLY_READY
NATIVE_STAGE SDL_CONTEXT_READY
NATIVE_STAGE APP_CREATOR_READY
NATIVE_STAGE APP_INITIALIZE_RESULT=0/1
NATIVE_STAGE MAIN_LOOP_RETURN
```

Esses marcadores foram compilados e verificados dentro de `libmain.so`, mas ainda é necessário confirmar no aparelho se aparecem no arquivo da sessão. O próximo APK deve manter esse diagnóstico e não removê-lo.

## Hipóteses prioritárias

Investigue nesta ordem:

1. `SDLActivity.nativeRunMain` falhando antes de chamar `SDL_main`.
2. Exceção Java/JNI ou retorno imediato do JNI.
3. `SDLWindowedAppContext::Initialize()` falhando.
4. Falha de criação da janela/superfície Android.
5. Falha no carregamento do Vulkan do sistema.
6. `ReXApp::SetupEnvironment()` ou `SetupPresentation()` retornando falso.
7. Falha do plugin `librexgpu-xenosd.so`.
8. Falha de abertura/leitura/descriptografia/descompressão do XEX.
9. Falha de mapeamento da memória guest.
10. Falha de imports, boundaries de funções ou lançamento do módulo recompilado.
11. Crash nativo SIGSEGV/SIGABRT sem exceção Java.

Não culpe PanVK sem evidência: o APK atual não empacota PanVK e usa o loader do sistema.

## Arquivos principais

Projeto:

```text
/home/ubuntu/pes2013_rex/
```

Arquivos importantes:

```text
/home/ubuntu/pes2013_rex/ANDROID_PORT_PLAN.md
/home/ubuntu/pes2013_rex/PORT_CONTINUITY.md
/home/ubuntu/pes2013_rex/pes2013_manifest.toml
/home/ubuntu/pes2013_rex/manual_functions.toml
/home/ubuntu/pes2013_rex/generated/rexglue.cmake
/home/ubuntu/pes2013_rex/generated/default/
home/ubuntu/pes2013_rex/android/app/src/main/cpp/CMakeLists.txt
/home/ubuntu/pes2013_rex/android/app/src/main/AndroidManifest.xml
/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/LauncherActivity.java
/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/GameActivity.java
/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/SDLActivity.java
/home/ubuntu/pes2013_rex/android/app/src/main/java/org/libsdl/app/SessionLogger.java
```

SDK/runtime:

```text
/home/ubuntu/work_rexglue_gcc14/src/
/home/ubuntu/work_rexglue_gcc14/include/
/home/ubuntu/work_rexglue_gcc14/cmake/
/home/ubuntu/work_rexglue_gcc14/thirdparty/
```

Arquivos do runtime mais relevantes:

```text
/home/ubuntu/work_rexglue_gcc14/src/ui/rex_app.cpp
/home/ubuntu/work_rexglue_gcc14/src/ui/windowed_app_main_sdl.cpp
/home/ubuntu/work_rexglue_gcc14/src/ui/vulkan/vulkan_instance.cpp
/home/ubuntu/work_rexglue_gcc14/src/core/logging.cpp
/home/ubuntu/work_rexglue_gcc14/include/rex/platform.h
/home/ubuntu/work_rexglue_gcc14/include/rex/platform/dynlib.h
```

Skill:

```text
/home/ubuntu/skills/pes2013-android-port/SKILL.md
```

## Build Android

O comando utilizado é:

```bash
cd /home/ubuntu/pes2013_rex/android
/home/ubuntu/.local/gradle/gradle-8.12/bin/gradle --no-daemon assembleDebug
```

Saída:

```text
/home/ubuntu/pes2013_rex/android/app/build/outputs/apk/debug/app-debug.apk
```

O último APK diagnóstico compilado tinha SHA-256:

```text
f2c8ca89bec5fe473d71d8f7a6c13e428d03aba8fee39d2cd7eead1dc6c99991
```

Depois de qualquer modificação:

1. verifique `build_rc=0`;
2. confirme `arm64-v8a`;
3. confirme `libmain.so`, `librexruntime*.so`, `librexgpu-xenosd.so`;
4. confirme ausência de bibliotecas x86;
5. confirme ausência de `libvulkan.so` empacotado;
6. execute `strings libmain.so | grep NATIVE_STAGE`;
7. gere SHA-256;
8. entregue o APK exato com link e hash.

## Diagnóstico sem computador

O usuário dispõe apenas do celular. Portanto, o aplicativo precisa gravar no próprio arquivo de sessão tudo que for possível. Não depender exclusivamente de `adb logcat`.

Local dos logs:

```text
/storage/emulated/0/Android/data/com.tokyogames.pes2013/files/logs/
```

A UI deve continuar mostrando o local do último log. Não apagar `game_data` ao instalar atualizações por cima.

Sem permissões privilegiadas, um APK comum não consegue capturar todo o logcat do sistema. Para falhas nativas graves, registre marcadores em arquivo próprio e, se necessário, oriente posteriormente a obtenção do logcat.

## Correções e recursos que ainda faltam

Depois de resolver a inicialização básica:

- criar diagnóstico Vulkan com nome da GPU, API version, `VkResult`, extensões e limites;
- registrar claramente as fases `SetupEnvironment`, `SetupPresentation`, `ConstructRuntime` e `LaunchModule`;
- registrar erro do XEX e dos módulos carregados;
- implementar fallback de erro visível no launcher em vez de tela preta silenciosa;
- implementar overlay Xbox 360 com analógicos, D-pad, A/B/X/Y, LB/RB, LT/RT, Start e Back;
- ligar overlay ao sistema de input SDL/ReXGlue, nunca escrever diretamente na memória guest;
- implementar resolução nativa, 720p, 900p, 1080p e personalizada;
- separar resolução interna e superfície Android;
- manter 16:9 com letterbox/pillarbox;
- invalidar cache de shaders por GPU, driver, resolução e configuração;
- adicionar créditos de Shisui Bahiano;
- testar em aparelho real Mali-G57 antes de chamar o port de funcional.

## Regras de segurança do desenvolvimento

- Não apagar ou sobrescrever `default.xex`.
- Não distribuir dados proprietários do jogo.
- Não usar AdrenoTools em Mali.
- Não empacotar driver Vulkan alternativo sem necessidade e autorização.
- Não afirmar que o jogo funciona antes de obter tela inicial no hardware real.
- Não mascarar erro com tela preta; registrar cada fase e mostrar mensagem legível.
- Não alterar muitas subsistemas ao mesmo tempo.
- A cada hipótese, gerar uma build identificável e registrar hash.

## Entrega esperada da próxima IA

A próxima IA deve entregar:

1. causa provável baseada em evidência do código e dos logs;
2. arquivos alterados e motivo de cada alteração;
3. comando de build reproduzível;
4. APK debug com SHA-256;
5. instruções de teste pelo celular;
6. interpretação esperada do novo log;
7. atualização da skill se o procedimento mudar;
8. relatório curto de progresso, sem inventar testes que não foram executados.

Comece lendo `PORT_CONTINUITY.md`, `ANDROID_PORT_PLAN.md` e `SKILL.md`. Depois audite o fluxo `LauncherActivity → GameActivity → SDLActivity.main → nativeRunMain → SDL_main → RunWindowedApp → ReXApp::OnInitialize`. Não avance para controles ou resolução até explicar o encerramento atual.
