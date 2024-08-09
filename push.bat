@echo off
REM 进入当前目录
cd /d %~dp0

REM 检查是否提供了提交消息
if "%~1"=="" (
    REM 没有提供提交消息，使用当前日期和时间
    set "commitMessage=Automated commit: %date% %time%"
) else (
    REM 使用提供的参数作为提交消息
    set "commitMessage=%~1"
)

REM 确保所有文件都被添加到暂存区
git add -A

REM 使用设置的提交消息
git commit -m "%commitMessage%"

REM 推送到远程仓库
git push origin master

REM 显示完成消息
echo All changes have been committed and pushed to the repository.
pause