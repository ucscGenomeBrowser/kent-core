kentSrc = ../..
include ../../inc/common.mk

GIT = git
GITUP = ${GIT} pull

user:
	${MAKE} doInstall destDir=${kentSrc}/hg/htdocs
	@echo ""
	@echo "You can now 'cd ~/kent/src/hg/htdocs' and run 'make' there"
	@echo ""
	@echo "To commit the change, run this to commit both the "
	@echo "Markdown and the HTML files:"
	@echo "git add ../mirrorManual.txt ../README.txt ../../hg/htdocs/mirror.html ../../hg/htdocs/goldenPath/help/mirrorManual.html"
	@echo "git commit"


doInstall:
	pandoc --ascii -f markdown-smart -t html5 -o ${destDir}/goldenPath/help/mirror.html ../README.txt --template staticPage.html -t staticPage.lua
	pandoc --ascii -f markdown-smart -t html5 -o ${destDir}/goldenPath/help/mirrorManual.html ../mirrorManual.txt --template staticPage.html -t staticPage.lua

testPandoc:
	@printf "Test the following path and commands look ok\n"
	@printf "pandoc path: %s\n" "`which pandoc`"
	@printf "pandoc --ascii -f markdown-smart -t html5 ../README.txt --template staticPage.html -t staticPage.lua\n" "${kentSrc}/hg/htdocs/"
	@printf "pandoc --ascii -f markdown-smart -t html5 ../mirrorManual.txt --template staticPage.html -t staticPage.lua\n" "${kentSrc}/hg/htdocs"
