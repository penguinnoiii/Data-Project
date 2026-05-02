#!/usr/bin/env bash

cat <<'EOF'
Git branch commands for this project

Check current branch
  git branch --show-current
  git status --short --branch

See all branches
  git branch
  git branch -a

Start from main
  git switch main
  git pull origin main

Create a new branch
  git switch -c feature/my-change

Work on the branch
  git status
  git add .
  git commit -m "Describe your change"
  git push -u origin feature/my-change

Compare with main
  git diff main...feature/my-change
  git log --oneline --graph --decorate --all

Merge back to main
  git switch main
  git pull origin main
  git merge feature/my-change
  git push origin main

Delete branch after merge
  git branch -d feature/my-change
  git push origin --delete feature/my-change

Current branch in this repo
  git switch feature/Matcha
  git status
  git add .
  git commit -m "Your message"
  git push
EOF
