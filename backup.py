import os
os.system("git add .; echo 'Files added to commit: \n {}\n'; git status; git commit -m '{}'; git push -u origin master".format(os.listdir(), input("Commit message: ").strip()))
