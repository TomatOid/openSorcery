cl /I.\include /I.\libs\SDL2\include /I.\libs\enet\include .\src\client.c .\src\generated_serialize.c SDL2.lib ws2_32.lib winmm.lib enet64.lib /link /SUBSYSTEM:CONSOLE /OUT:client.exe 
pause
