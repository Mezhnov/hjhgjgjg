package com.example.playstoreclone.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.R
import com.example.playstoreclone.databinding.ItemScreenshotBinding

class ScreenshotAdapter(
    private val screenshots: List<String>,
    private val onScreenshotClick: (Int) -> Unit
) : RecyclerView.Adapter<ScreenshotAdapter.ViewHolder>() {

    inner class ViewHolder(private val binding: ItemScreenshotBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(screenshotUrl: String, position: Int) {
            Glide.with(itemView.context)
                .load(screenshotUrl)
                .placeholder(R.drawable.bg_screenshot)
                .transform(RoundedCorners(16))
                .into(binding.ivScreenshot)

            binding.root.setOnClickListener { onScreenshotClick(position) }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = ItemScreenshotBinding.inflate(
            LayoutInflater.from(parent.context), parent, false
        )
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(screenshots[position], position)
    }

    override fun getItemCount() = screenshots.size
}
