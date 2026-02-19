package com.example.playstoreclone

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.example.playstoreclone.adapters.ViewPagerAdapter
import com.example.playstoreclone.databinding.ActivityMainBinding
import com.google.android.material.tabs.TabLayoutMediator

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var viewPagerAdapter: ViewPagerAdapter

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupViewPager()
        setupBottomNavigation()
        setupSearchBar()
    }

    private fun setupViewPager() {
        viewPagerAdapter = ViewPagerAdapter(this)
        binding.viewPager.adapter = viewPagerAdapter

        // Connect TabLayout with ViewPager2
        TabLayoutMediator(binding.tabLayout, binding.viewPager) { tab, position ->
            tab.text = viewPagerAdapter.getTabTitle(position)
        }.attach()
    }

    private fun setupBottomNavigation() {
        binding.bottomNavigation.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_games -> {
                    // Switch to Games section
                    updateTabsForSection("games")
                    true
                }
                R.id.nav_apps -> {
                    // Switch to Apps section
                    updateTabsForSection("apps")
                    true
                }
                R.id.nav_movies -> {
                    // Switch to Movies section
                    updateTabsForSection("movies")
                    true
                }
                R.id.nav_books -> {
                    // Switch to Books section
                    updateTabsForSection("books")
                    true
                }
                else -> false
            }
        }

        // Set Games as default selected
        binding.bottomNavigation.selectedItemId = R.id.nav_games
    }

    private fun updateTabsForSection(section: String) {
        // Here you can update the tabs based on the selected section
        // For now, we keep the same tabs for all sections
        binding.viewPager.setCurrentItem(0, false)
    }

    private fun setupSearchBar() {
        // Search bar click handling
        // You can add an OnClickListener to open a SearchActivity
    }
}
