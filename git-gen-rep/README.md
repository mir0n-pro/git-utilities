
git_gen_rep.bat (a windows batch file to to run git bash script)
git_gen_rep.sh (git bash script)

just copy both files onto
c:\Program Files\Git\cmd

How to use:
From local repo run the command:
git_gen_rep  [from [till]]

Where "from" "till" are optional: tag or commit hash, or pseudo mark: like "HEAD", "HEAD~2" &c.

When "till" is omitted it works up to HEAD.

When both are omitted it works only for last commit, same as :
report_gen_tag  HEAD~1 HEAD

When till is omitted it works up to HEAD.

Please note: the report file will be placed on .\doc\reports\ folder

-----------------------------------------------------------------------------------------------------
If you have no access to the Git\cmd you can place both files onto any directory available thru PATH.

Alternative way : place both files to any accessible folder, and run the batch file from there.
example: staying in the local repository, and having the batch and bash files in the c:\mycmd folder,
you need to call
c:\mycmd\git_gen_rep

Also, you can use the root of your Git local repositories to install.
In this case you just need to run "..\git_gen_rep"


