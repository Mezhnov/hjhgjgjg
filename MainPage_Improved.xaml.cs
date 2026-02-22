namespace MauiApp3
{
    public partial class MainPage : ContentPage
    {
        public MainPage()
        {
            InitializeComponent();
            InitializeMockData();
        }

        private void InitializeMockData()
        {
            SceneObjectsList.ItemsSource = new List<SceneObject>
            {
                // Lighting Folder
                new SceneObject { Name = "Lighting", Icon = "📁", IsFolder = true, IsExpanded = true },
                new SceneObject { Name = "   DirectionalLight", Icon = "☀️", IsSelected = false },
                new SceneObject { Name = "   SkyLight", Icon = "🌤", IsSelected = false },
                new SceneObject { Name = "   SkyAtmosphere", Icon = "🌐", IsSelected = false },
                
                // Environment
                new SceneObject { Name = "Environment", Icon = "📁", IsFolder = true, IsExpanded = true },
                new SceneObject { Name = "   ExponentialHeightFog", Icon = "🌫", IsSelected = false },
                new SceneObject { Name = "   VolumetricCloud", Icon = "☁️", IsSelected = false },
                new SceneObject { Name = "   PostProcessVolume", Icon = "🎨", IsSelected = false },
                
                // Geometry
                new SceneObject { Name = "Geometry", Icon = "📁", IsFolder = true, IsExpanded = false },
                new SceneObject { Name = "   Floor", Icon = "⬜", IsSelected = false },
                new SceneObject { Name = "   Walls", Icon = "🧱", IsSelected = false },
                
                // Props
                new SceneObject { Name = "Props", Icon = "📁", IsFolder = true, IsExpanded = true },
                new SceneObject { Name = "   SM_Couch", Icon = "🛋", IsSelected = true },
                new SceneObject { Name = "   SM_Table", Icon = "🪑", IsSelected = false, IsLocked = true },
                new SceneObject { Name = "   SM_Lamp", Icon = "💡", IsSelected = false },
                
                // Characters
                new SceneObject { Name = "Characters", Icon = "📁", IsFolder = true, IsExpanded = false },
                new SceneObject { Name = "   BP_PlayerCharacter", Icon = "🧍", IsSelected = false },
                
                // Cameras
                new SceneObject { Name = "CameraActor", Icon = "🎥", IsSelected = false },
                new SceneObject { Name = "PlayerStart", Icon = "🚩", IsSelected = false },
            };
        }

        private void OnSaveClicked(object sender, EventArgs e)
        {
            DisplayAlert("Unreal Engine", "All assets saved successfully!", "OK");
        }

        private void OnPlayClicked(object sender, EventArgs e)
        {
            DisplayAlert("Play", "Starting Play in Editor...", "OK");
        }

        public class SceneObject
        {
            public string? Name { get; set; }
            public string? Icon { get; set; }
            public bool IsFolder { get; set; } = false;
            public bool IsExpanded { get; set; } = false;
            public bool IsSelected { get; set; } = false;
            public bool IsLocked { get; set; } = false;
            public bool IsVisible { get; set; } = true;
        }
    }
}