
## Code style

We use astyle to format code

Look at git_hooks/astyle-options.txt,
you can use the pre-commit hook in the same dir,but you need to install cppcheck and astyle.

## How to Write a Git Commit Message

We use [this standard](https://chris.beams.io/posts/git-commit/).

You can use the commit-msg git hook in  git_hooks/, but you need to install python 3.


## How to make a pull request

- fork the alibaba/CicadaPlayer git
 
- clone the CicadaPlayer git into your develop pc
```bash
git clone git@github.com:yourname/CicadaPlayer.git
```

- add the alibaba remote and fetch
```bash
cd CicadaPlayer
git remote add alibaba git://github.com/alibaba/CicadaPlayer.git
git fetch alibaba
```

- checkout a new branch form alibaba develop
```bash
git checkout alibaba develop -b develop
git checkout -b topic alibaba/develop
```

- rebase your change
```bash
git checkout develop
git pull alibaba develop

git checkout topic
git rebase develop
```

- after resolve conflict , push to origin
```bash
git push origin topic:topic
```

- make a pull request to alibaba/CicadaPlayer, 
this will trigger a travis build, unit test and coverage test,
your pull request will not be merged if build err, coverage descend or the cla is not be agreed.






