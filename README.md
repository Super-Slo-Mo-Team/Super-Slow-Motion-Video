# Super-Slow-Motion-Video

## Workflow:

Team members will use the following process to develop a feature or bug fix.

1. Create a branch using the following naming convention:
   * <initials>‐<name‐of‐feature>  e.g.  nd‐dev‐standards
2. When you are ready for feedback on your changes, even if they are not ready to be fully reviewed, open a pull request.
3. When your changes are ready for review, mark your pull request with the yellow  review  label. Strive to resolve merge conflicts with  master  before asking for review.
4. Pull requests require at least one other teammate to review and sign off via GitHub pull request approval. Feel free to add an  LGTM ,   ﴾type  :ship: ﴿, or other similar comment in the comment thread so the pull request creator is notified about your approval. Pull requests with systemic or sweeping changes require all teammates to review and sign off. Code review responsibilities should be divided among all teammates.
5. The pull request creator will make any reasonable changes requested by reviewers until the pull request is approved.
6. When a pull request passes the review requirements and is ready to merge, remove the  review  label and add the blue ship  label.
7. The pull request creator will ship the feature into  master  with a squash commit via GitHub. Commit messages should, at a minimum, contain a concise but descriptive explanation of the changes.

## Optional
  * Commit messages can contain references to issue numbers for auto‐resolution. e.g.  Closes #10 
  * Delete your feature branch after it is merged into  master . In general, we should keep feature branches for 1‐2 weeks after merging in case we need to reference them post‐merge.
  * Consider resolving merge conflicts with the  git town‐sync  command of git‐town, a tool that simplifies Git workflows.
  
## Edge Cases
  * If a pull request is accidentally merged without squashing, the pull request creator is responsible for reverting the offending commits with  git revert  or an equivalent command. Since the original pull request should have already passed review, the creator has authority to open a new pull request with the original changes, mark it with the  ship label, and immediately squash commit the changes into  master .
  * If a pull request is accidentally merged before review, the pull request creator is responsible for reverting the offending commits with  git revert  or an equivalent, reopening a pull request, and marking it with the  review  label. ﴾This should happen rarely since our project settings don't allow merging via the web unless the pull request has been reviewed and approved.﴿
  
