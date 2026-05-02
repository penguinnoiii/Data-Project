# Git Branch Commands For This Project

Use this file as a quick reference when working with `main` and a feature branch.

## Check Your Current Branch

```bash
git branch --show-current
git status --short --branch
```

## See All Branches

```bash
git branch
git branch -a
```

## Start From `main`

```bash
git switch main
git pull origin main
```

## Create A New Branch

```bash
git switch -c feature/my-change
```

## Work On The Branch

```bash
git status
git add .
git commit -m "Describe your change"
git push -u origin feature/my-change
```

## Compare Branch With `main`

```bash
git diff main...feature/my-change
git log --oneline --graph --decorate --all
```

## Merge Branch Back To `main`

```bash
git switch main
git pull origin main
git merge feature/my-change
git push origin main
```

## Delete The Branch After Merge

```bash
git branch -d feature/my-change
git push origin --delete feature/my-change
```

## Commands For The Current Branch In This Repo

The branch already in this project is:

```bash
git switch feature/Matcha
```

If you want to keep using it:

```bash
git status
git add .
git commit -m "Your message"
git push
```
