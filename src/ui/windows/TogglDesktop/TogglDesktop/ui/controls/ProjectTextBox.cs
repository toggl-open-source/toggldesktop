namespace TogglDesktop
{
    class ProjectTextBox : ExtendedTextBox
    {
        public void SetText(string project, string task)
        {
            var resultText = project + (task.IsNullOrEmpty() ? "" : $" - {task}");
            this.SetText(resultText);
        }
    }
}