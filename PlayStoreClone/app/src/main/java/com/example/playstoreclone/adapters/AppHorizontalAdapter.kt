package com.example.playstoreclone.adapters

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.bumptech.glide.load.resource.bitmap.RoundedCorners
import com.example.playstoreclone.R
import com.example.playstoreclone.databinding.ItemAppHorizontalBinding
import com.example.playstoreclone.models.AppModel

class AppHorizontalAdapter(
    private val apps: List<AppModel>,
    private val onAppClick: (AppModel) -> Unit
) : RecyclerView.Adapter<AppHorizontalAdapter.ViewHolder>() {

    inner class ViewHolder(private val binding: ItemAppHorizontalBinding) :
        RecyclerView.ViewHolder(binding.root) {

        fun bind(app: AppModel) {
            binding.apply {
                tvAppName.text = app.name
                tvRating.text = app.rating.toString()

                // Load app icon from URL using Glide
                Glide.with(itemView.context)
                    .load(app.iconUrl)
                    .placeholder(R.drawable.ic_placeholder)
                    .transform(RoundedCorners(40))
                    .into(ivAppIcon)

                root.setOnClickListener { onAppClick(app) }
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val binding = ItemAppHorizontalBinding.inflate(
            LayoutInflater.from(parent.context), parent, false
        )
        return ViewHolder(binding)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bind(apps[position])
    }

    override fun getItemCount() = apps.size
}
