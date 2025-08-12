pause format src?
clang-format --style=Google -i *.cpp *.h
rem clang-format --style=Google -i DigitalIO/*.h
rem clang-format --style=Google -i DigitalIO/boards/*.h
clang-format --style=Google -i common/*.cpp common/*.h
clang-format --style=Google -i ExFatLib/*.cpp ExFatLib/*.h
clang-format --style=Google -i FatLib/*.cpp FatLib/*.h
clang-format --style=Google -i FsLib/*.cpp FsLib/*.h
clang-format --style=Google -i iostream/*.cpp iostream/*.h
clang-format --style=Google -i SdCard/*.cpp SdCard/*.h
rem clang-format --style=Google -i SdCard/Rp2040Sdio/DbgLogMsg.h SdCard/Rp2040Sdio/PioDbgInfo.h
rem clang-format --style=Google -i SdCard/Rp2040Sdio/PioSdioCard.h SdCard/Rp2040Sdio/Rp2040SdioConfig.h
clang-format --style=Google -i SdCard/PioSdio/*.cpp SdCard/PioSdio/*.h
clang-format --style=Google -i SdCard/SdSpiCard/*.cpp SdCard/SdSpiCard/*.h
clang-format --style=Google -i SdCard/SdSpiCard/SpiDriver/*.cpp SdCard/SdSpiCard/SpiDriver/*.h
clang-format --style=Google -i SdCard/TeensySdio/*.cpp SdCard/TeensySdio/*.h
pause
