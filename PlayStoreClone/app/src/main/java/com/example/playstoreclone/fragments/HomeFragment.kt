package com.example.playstoreclone.fragments

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.example.playstoreclone.AppDetailActivity
import com.example.playstoreclone.R
import com.example.playstoreclone.adapters.*
import com.example.playstoreclone.databinding.FragmentHomeBinding
import com.example.playstoreclone.models.AppModel
import com.example.playstoreclone.models.BannerModel
import com.example.playstoreclone.models.CategoryModel
import com.google.android.material.tabs.TabLayoutMediator

class HomeFragment : Fragment() {

    private var _binding: FragmentHomeBinding? = null
    private val binding get() = _binding!!

    private var tabName: String = "For you"

    companion object {
        private const val ARG_TAB_NAME = "tab_name"

        fun newInstance(tabName: String): HomeFragment {
            return HomeFragment().apply {
                arguments = Bundle().apply {
                    putString(ARG_TAB_NAME, tabName)
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            tabName = it.getString(ARG_TAB_NAME, "For you")
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentHomeBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        setupBanner()
        setupRecommendedApps()
        setupTopCharts()
        setupCategories()
        setupEditorsChoice()
    }

    private fun setupBanner() {
        val banners = getSampleBanners()
        val adapter = BannerAdapter(banners) { banner ->
            // Handle banner click - navigate to app detail
        }
        binding.bannerViewPager.adapter = adapter

        // Setup indicator
        TabLayoutMediator(binding.bannerIndicator, binding.bannerViewPager) { _, _ -> }.attach()
    }

    private fun setupRecommendedApps() {
        val apps = getSampleApps()
        val adapter = AppHorizontalAdapter(apps) { app ->
            navigateToAppDetail(app)
        }
        binding.rvRecommended.adapter = adapter
    }

    private fun setupTopCharts() {
        val apps = getSampleApps().take(5)
        val adapter = AppListAdapter(apps) { app ->
            navigateToAppDetail(app)
        }
        binding.rvTopCharts.adapter = adapter
    }

    private fun setupCategories() {
        val categories = getSampleCategories()
        val adapter = CategoryAdapter(categories) { category ->
            // Handle category click
        }
        binding.rvCategories.adapter = adapter
    }

    private fun setupEditorsChoice() {
        val apps = getSampleApps()
        val adapter = AppLargeAdapter(apps) { app ->
            navigateToAppDetail(app)
        }
        binding.rvEditorsChoice.adapter = adapter
    }

    private fun navigateToAppDetail(app: AppModel) {
        val intent = Intent(requireContext(), AppDetailActivity::class.java).apply {
            putExtra("app_id", app.id)
            putExtra("app_name", app.name)
            putExtra("app_developer", app.developer)
            putExtra("app_icon", app.iconUrl)
            putExtra("app_rating", app.rating)
            putExtra("app_description", app.description)
        }
        startActivity(intent)
    }

    // Sample data - Replace with real data from API or database
    private fun getSampleBanners(): List<BannerModel> {
        return listOf(
            BannerModel(
                id = "1",
                title = "Featured Game",
                subtitle = "New adventure awaits",
                imageUrl = "https://picsum.photos/800/400?random=1",
                appIconUrl = "https://picsum.photos/200?random=10",
                appId = "1"
            ),
            BannerModel(
                id = "2",
                title = "Editor's Pick",
                subtitle = "Best productivity app",
                imageUrl = "https://picsum.photos/800/400?random=2",
                appIconUrl = "https://picsum.photos/200?random=11",
                appId = "2"
            ),
            BannerModel(
                id = "3",
                title = "Top Chart",
                subtitle = "Most downloaded this week",
                imageUrl = "https://picsum.photos/800/400?random=3",
                appIconUrl = "https://picsum.photos/200?random=12",
                appId = "3"
            )
        )
    }

    private fun getSampleApps(): List<AppModel> {
        return listOf(
            AppModel(
                id = "1",
                name = "Super Adventure Game",
                developer = "GameStudio Inc.",
                iconUrl = "https://picsum.photos/200?random=20",
                rating = 4.5f,
                reviewCount = "1.2M",
                downloadCount = "100M+",
                size = "85 MB",
                category = "Action",
                description = "An amazing adventure game with stunning graphics and exciting gameplay. Explore vast worlds, battle enemies, and discover hidden treasures.",
                whatsNew = "• New levels added\n• Bug fixes\n• Performance improvements",
                ageRating = "12+",
                screenshots = listOf(
                    "https://picsum.photos/400/800?random=30",
                    "https://picsum.photos/400/800?random=31",
                    "https://picsum.photos/400/800?random=32"
                ),
                tags = listOf("Action", "Adventure", "Single player"),
                containsAds = true,
                inAppPurchases = true,
                featuredImageUrl = "https://picsum.photos/600/300?random=40"
            ),
            AppModel(
                id = "2",
                name = "Photo Editor Pro",
                developer = "Creative Apps",
                iconUrl = "https://picsum.photos/200?random=21",
                rating = 4.7f,
                reviewCount = "500K",
                downloadCount = "50M+",
                size = "45 MB",
                category = "Photography",
                description = "Professional photo editing tools at your fingertips. Create stunning images with filters, effects, and advanced editing features.",
                whatsNew = "• New filters\n• AI enhancement\n• Export improvements",
                ageRating = "3+",
                featuredImageUrl = "https://picsum.photos/600/300?random=41"
            ),
            AppModel(
                id = "3",
                name = "Music Player Ultimate",
                developer = "Audio Labs",
                iconUrl = "https://picsum.photos/200?random=22",
                rating = 4.3f,
                reviewCount = "800K",
                downloadCount = "80M+",
                size = "25 MB",
                category = "Music & Audio",
                description = "The ultimate music player with equalizer, bass boost, and visualizer. Support for all audio formats.",
                whatsNew = "• New equalizer presets\n• Improved UI\n• Bug fixes",
                ageRating = "3+",
                featuredImageUrl = "https://picsum.photos/600/300?random=42"
            ),
            AppModel(
                id = "4",
                name = "Fitness Tracker",
                developer = "Health Apps Co.",
                iconUrl = "https://picsum.photos/200?random=23",
                rating = 4.6f,
                reviewCount = "300K",
                downloadCount = "30M+",
                size = "35 MB",
                category = "Health & Fitness",
                description = "Track your workouts, count steps, and monitor your health. Achieve your fitness goals with our comprehensive tracking app.",
                whatsNew = "• New workout modes\n• Sleep tracking\n• Heart rate zones",
                ageRating = "3+",
                featuredImageUrl = "https://picsum.photos/600/300?random=43"
            ),
            AppModel(
                id = "5",
                name = "Weather Now",
                developer = "MeteoApps",
                iconUrl = "https://picsum.photos/200?random=24",
                rating = 4.4f,
                reviewCount = "1M",
                downloadCount = "100M+",
                size = "15 MB",
                category = "Weather",
                description = "Accurate weather forecasts with beautiful widgets. Get hourly and daily forecasts for any location.",
                whatsNew = "• Improved accuracy\n• New widgets\n• Radar maps",
                ageRating = "3+",
                featuredImageUrl = "https://picsum.photos/600/300?random=44"
            ),
            AppModel(
                id = "6",
                name = "Social Network",
                developer = "Social Inc.",
                iconUrl = "https://picsum.photos/200?random=25",
                rating = 4.2f,
                reviewCount = "5M",
                downloadCount = "500M+",
                size = "65 MB",
                category = "Social",
                description = "Connect with friends, share moments, and discover new content. The best way to stay connected.",
                whatsNew = "• Stories feature\n• Dark mode\n• Performance boost",
                ageRating = "12+",
                containsAds = true,
                featuredImageUrl = "https://picsum.photos/600/300?random=45"
            )
        )
    }

    private fun getSampleCategories(): List<CategoryModel> {
        return listOf(
            CategoryModel("1", "Action", R.drawable.ic_games, R.color.category_games),
            CategoryModel("2", "Puzzle", R.drawable.ic_games, R.color.category_apps),
            CategoryModel("3", "Racing", R.drawable.ic_games, R.color.category_movies),
            CategoryModel("4", "Strategy", R.drawable.ic_games, R.color.category_books),
            CategoryModel("5", "Sports", R.drawable.ic_games, R.color.primary),
            CategoryModel("6", "RPG", R.drawable.ic_games, R.color.accent_light)
        )
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
