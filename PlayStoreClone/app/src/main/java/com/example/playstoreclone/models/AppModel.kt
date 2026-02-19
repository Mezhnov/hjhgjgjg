package com.example.playstoreclone.models

data class AppModel(
    val id: String,
    val name: String,
    val developer: String,
    val iconUrl: String,
    val rating: Float,
    val reviewCount: String,
    val downloadCount: String,
    val size: String,
    val category: String,
    val description: String,
    val whatsNew: String,
    val ageRating: String,
    val screenshots: List<String> = emptyList(),
    val tags: List<String> = emptyList(),
    val containsAds: Boolean = false,
    val inAppPurchases: Boolean = false,
    val price: String = "Free",
    val featuredImageUrl: String = ""
)
