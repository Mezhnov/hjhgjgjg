package com.example.playstoreclone

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.adapters.AppHorizontalAdapter
import com.example.playstoreclone.adapters.ScreenshotAdapter
import com.example.playstoreclone.databinding.ActivityAppDetailBinding
import com.example.playstoreclone.models.AppModel

class AppDetailActivity : AppCompatActivity() {

    private lateinit var binding: ActivityAppDetailBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityAppDetailBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        loadAppDetails()
        setupScreenshots()
        setupSimilarApps()
        setupInstallButton()
    }

    private fun setupToolbar() {
        binding.toolbar.setNavigationOnClickListener {
            onBackPressedDispatcher.onBackPressed()
        }
    }

    private fun loadAppDetails() {
        // Get data from intent
        val appName = intent.getStringExtra("app_name") ?: "App Name"
        val appDeveloper = intent.getStringExtra("app_developer") ?: "Developer"
        val appIconUrl = intent.getStringExtra("app_icon") ?: ""
        val appRating = intent.getFloatExtra("app_rating", 4.5f)
        val appDescription = intent.getStringExtra("app_description") ?: "App description"

        // Set data to views
        binding.apply {
            tvAppName.text = appName
            tvDeveloper.text = appDeveloper
            tvRating.text = appRating.toString()
            tvBigRating.text = appRating.toString()
            tvDescription.text = appDescription
            tvReviewCount.text = "1M reviews"
            tvDownloads.text = "100M+"
            tvAgeRating.text = "12+"
            tvWhatsNew.text = "• Bug fixes and improvements\n• New features added\n• Performance optimizations"
            tvTotalReviews.text = "1,234,567"
            ratingBar.rating = appRating

            // Load app icon
            Glide.with(this@AppDetailActivity)
                .load(appIconUrl)
                .placeholder(R.drawable.ic_placeholder)
                .transform(RoundedCorners(40))
                .into(ivAppIcon)
        }
    }

    private fun setupScreenshots() {
        val screenshots = listOf(
            "https://picsum.photos/400/800?random=50",
            "https://picsum.photos/400/800?random=51",
            "https://picsum.photos/400/800?random=52",
            "https://picsum.photos/400/800?random=53",
            "https://picsum.photos/400/800?random=54"
        )

        val adapter = ScreenshotAdapter(screenshots) { position ->
            // Handle screenshot click - could open fullscreen viewer
            Toast.makeText(this, "Screenshot ${position + 1}", Toast.LENGTH_SHORT).show()
        }
        binding.rvScreenshots.adapter = adapter
    }

    private fun setupSimilarApps() {
        val similarApps = listOf(
            AppModel(
                id = "s1",
                name = "Similar App 1",
                developer = "Developer",
                iconUrl = "https://picsum.photos/200?random=60",
                rating = 4.3f,
                reviewCount = "100K",
                downloadCount = "10M+",
                size = "50 MB",
                category = "Games",
                description = "",
                whatsNew = "",
                ageRating = "12+"
            ),
            AppModel(
                id = "s2",
                name = "Similar App 2",
                developer = "Developer",
                iconUrl = "https://picsum.photos/200?random=61",
                rating = 4.5f,
                reviewCount = "200K",
                downloadCount = "20M+",
                size = "60 MB",
                category = "Games",
                description = "",
                whatsNew = "",
                ageRating = "12+"
            ),
            AppModel(
                id = "s3",
                name = "Similar App 3",
                developer = "Developer",
                iconUrl = "https://picsum.photos/200?random=62",
                rating = 4.1f,
                reviewCount = "50K",
                downloadCount = "5M+",
                size = "40 MB",
                category = "Games",
                description = "",
                whatsNew = "",
                ageRating = "12+"
            ),
            AppModel(
                id = "s4",
                name = "Similar App 4",
                developer = "Developer",
                iconUrl = "https://picsum.photos/200?random=63",
                rating = 4.7f,
                reviewCount = "300K",
                downloadCount = "30M+",
                size = "70 MB",
                category = "Games",
                description = "",
                whatsNew = "",
                ageRating = "12+"
            )
        )

        val adapter = AppHorizontalAdapter(similarApps) { app ->
            // Handle similar app click
            Toast.makeText(this, "Clicked: ${app.name}", Toast.LENGTH_SHORT).show()
        }
        binding.rvSimilarApps.adapter = adapter
    }

    private fun setupInstallButton() {
        binding.btnInstall.setOnClickListener {
            Toast.makeText(this, "Installing...", Toast.LENGTH_SHORT).show()
            binding.btnInstall.text = "Installing..."
            binding.btnInstall.isEnabled = false

            // Simulate installation
            binding.btnInstall.postDelayed({
                binding.btnInstall.text = getString(R.string.open)
                binding.btnInstall.isEnabled = true
            }, 2000)
        }
    }
}
