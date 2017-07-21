@echo off
timeout /T 2 /NOBREAK > NUL

start cmd /K "title HV && server.exe HV 30401"
cmdow "*HV*" /MOV 0 0
cmdow "*HV*" /SIZ 640 350

start cmd /K "title micra && server.exe micra 30402"
cmdow.exe "*micra*" /MOV 0 335
cmdow.exe "*micra*" /SIZ 640 350

start cmd /K "title chiller && server.exe chiller 30403"
cmdow.exe "*chiller*" /MOV 0 670
cmdow.exe "*chiller*" /SIZ 640 350

start cmd /K "title shutter1 && server.exe shutter1 30501"
cmdow.exe "*shutter1*" /MOV 640 0
cmdow.exe "*shutter1*" /SIZ 640 350

start cmd /K "title shutter2 && server.exe shutter2 30502"
cmdow.exe "*shutter2*" /MOV 640 335
cmdow.exe "*shutter2*" /SIZ 640 350

start cmd /K "title shutter3 && server.exe shutter3 30503"
cmdow.exe "*shutter3*" /MOV 640 670
cmdow.exe "*shutter3*" /SIZ 640 350

start cmd /K "title parker1 && server.exe parker1 30601"
cmdow.exe "*parker1*" /MOV 1280 0
cmdow.exe "*parker1*" /SIZ 640 350

start cmd /K "title parker2 && server.exe parker2 30602"
cmdow.exe "*parker2*" /MOV 1280 335
cmdow.exe "*parker2*" /SIZ 640 350

start cmd /K "title parker3 && server.exe parker3 30603"
cmdow.exe "*parker3*" /MOV 1280 670
cmdow.exe "*parker3*" /SIZ 640 350

start cmd /K "title newportdelay && server.exe newportdelay 30604"
cmdow.exe "*newportdelay*" /MOV 1920 0
cmdow.exe "*newportdelay*" /SIZ 640 350

start cmd /K "title rotation && server.exe rotation 30605"
cmdow.exe "*rotation*" /MOV 1920 335
cmdow.exe "*rotation*" /SIZ 640 350

/c
