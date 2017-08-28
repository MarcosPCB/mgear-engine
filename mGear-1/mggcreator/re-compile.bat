@echo off
cls

echo Re-compiling mainly MGGs

mggcreator -o mEngUI.mgg -p engUI
mggcreator -o ENGINEER.MGG -p engin
mggcreator -o STAGE1.MGG -p stage1
mggcreator -o STAGE2.MGG -p stage2
mggcreator -o STAGE3.MGG -p stage3
mggcreator -o glacius.mgg -p glacius
mggcreator -o sektor.mgg -p sektor
mggcreator -o gstage.mgg -p gstage